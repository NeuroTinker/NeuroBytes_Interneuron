#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/exti.h>

#include "comm.h"
#include "HAL.h"
#include "neuron.h"

#define BLINK_TIME			100
#define DATA_TIME			10
#define DEND_PING_TIME		200 // 1000 ms
#define	NID_PING_TIME		200 // 1000 ms
#define SEND_PING_TIME		80 // 80
#define BUTTON_PRESS_TIME	2

static uint32_t fingerprint[3] __attribute__((section (".fingerprint"))) __attribute__ ((__used__)) = {
	1, // device id
	1, // firmware version
	0  // unique id
};

int main(void)
{
	uint8_t		i;

	// counters
	uint32_t	blink_time = 0; // counter for LED blink due to NID command
	uint16_t	data_time = 0; // counter for sending data to NID
	uint16_t	send_ping_time = 0; // counter for sending a downstream ping
	uint16_t	fire_delay_time = 0;
	uint8_t		fire_flag = 0;

	// button debounce variables
	uint16_t	button_press_time = 0; 
	uint8_t		button_armed = 0;
	uint16_t	button_status = 0;

	// current channel used to communicate to NID (e.g. CH. 1). 0 if neuron has not been selected by NID
	uint32_t	nid_channel = 0b000;

	uint32_t	message = 0; // staging variable for constructing messages to send to the communications routine

	// initialize neuron
	neuron_t 	neuron;
	neuronInit(&neuron);

	// initialize communication buffers
	commInit();

	// setup hardware
	clock_setup();
	systick_setup(100); // systick in microseconds
	gpio_setup();
	tim_setup();

	// main processing routine	
	for(;;)
	{
		if (main_tick == 1){
			// main tick every 5 ms
			main_tick = 0;
			
			// check to see if nid ping hasn't been received in last NID_PING_TIME ticks
			if (nid_ping_time++ > NID_PING_TIME){
				// nid no longer connected
				nid_keep_alive = NID_PING_KEEP_ALIVE; // reset nid_keep_alive
				nid_pin = 0; // clear the nid pin
				nid_pin_out = 0;
			}
			
			// send a downstream ping every SEND_PING_TIME ticks
			if (send_ping_time++ > SEND_PING_TIME){
				// send downstream ping through axon
				addWrite(DOWNSTREAM_BUFF, DEND_PING);
				send_ping_time = 0;
			}


			/*
				nid_channel is the current channel, if any, that the NeuroByte is using to communicate
				with the NID. nid_channel should be cleared when NID tries to set a new NeuroByte to 
				identify_channel.

				The communication routine sets identify_time to zero when a new identify command is received.
			*/

			// check for clear channel command
			if (identify_time < IDENTIFY_TIME){
				identify_time += 1;
				if (identify_channel == 0){
					// setting identify channel 0 clears identify_channel
					nid_channel = 0;
				} else if (identify_channel == nid_channel && identify_time == 0){
					// clear nid_channel if NID is trying to set a new NeuroByte to the current nid_channel
					nid_channel = 0;
				}
			}

			// check identify button
			button_status = gpio_get(PORT_IDENTIFY, PIN_IDENTIFY);
			//button_status |= PIN_IDENTIFY;

			// if identify button is pressed and identify_time < IDENTIFY_TIME (i.e. NID sent 'identify'' message), set new nid_channel
			if (button_status == 0){
				// debounce
				if (button_press_time++ >= BUTTON_PRESS_TIME){
					button_armed = 1;
					button_press_time = 0;
				}
			} else if (button_armed == 1){
				if (identify_time < IDENTIFY_TIME){
					nid_channel = identify_channel;
				} else{
					// temporarily use identify button also as an impulse button
					neuron.fire_potential += 11000;
					//neuron.leaky_current += 20;
				}
				button_armed = 0;
			} else{
				button_press_time = 0;
			}
			
			// send current membrane potential to NID if currently identified by NID
			if (nid_channel != 0){
				// send data every DATA_TIME ticks
				if (data_time++ > DATA_TIME){
					data_time = 0;
					message = DATA_MESSAGE | (uint16_t) neuron.potential | (nid_channel << 19) | (nid_keep_alive << 22);
					addWrite(NID_BUFF,message);
				}
			}

			getFingerprint();
			
			/*
				Check dendrites for pings and adjut pins accordingly.
				Also check dendrites for pulses and calculate new membrane potential.
			*/
			checkDendrites(&neuron);
			
			// decay old dendrite contributions to the membrane potential
			dendriteDecayStep(&neuron);
			// decay the firing potential
			membraneDecayStep(&neuron);

			// current membrane potential comes from dendrites and any left over firing potential
			neuron.potential = calcNeuronPotential(&neuron);
			neuron.potential += neuron.fire_potential;
			neuron.fire_potential += neuron.leaky_current;

			// if membrane potential is greater than threshold, fire
			if (neuron.potential > MEMBRANE_THRESHOLD){
				// fire for determined pulse width
				neuron.state = FIRE;
				neuron.fire_potential = HYPERPOLARIZATION;
				neuron.fire_time = PULSE_LENGTH;
				for (i=0; i<DENDRITE_COUNT; i++){
					neuron.dendrites[i].current_value = 0;
					neuron.dendrites[i].state = OFF;
				}
				// send downstream pulse
				fire_delay_time = FIRE_DELAY_TIME;
				fire_flag = 1;
			}

			if (fire_delay_time > 0){
				fire_delay_time -= 1;
			} else if (fire_flag == 1){
				fire_flag = 0;
				addWrite(DOWNSTREAM_BUFF, PULSE_MESSAGE);
			}

			/*
				LED is either:
				-blinking due to NID message (blink_flag is set)
				-full white if neuron is firing
				-in an RGB state mapped to membrane potential
			*/
			if (blink_flag != 0){
				setLED(200,0,300);
				blink_time = 1;
				blink_flag = 0;
			} else if (blink_time > 0){
				if (++blink_time == BLINK_TIME){
					setLED(200,0,0);
					blink_time = 0;
				}
			} else if (neuron.state == FIRE){
				neuron.fire_time -= 1;
				if (neuron.fire_time == 0){
					neuron.state = INTEGRATE;
				}
				LEDFullWhite();
			} else if (neuron.state == INTEGRATE){
				if (neuron.potential > 10000){
					setLED(200,0,0);
				} else if (neuron.potential > 0){
					setLED(neuron.potential / 50, 200 - (neuron.potential / 50), 0);
				} else if (neuron.potential < -10000){
					setLED(0,0, 200);
				} else if (neuron.potential < 0){
					setLED(0, 200 + (neuron.potential / 50), -1 * neuron.potential / 50);
				} else{
					setLED(0,200,0);
				}
			}
		}
	}
}