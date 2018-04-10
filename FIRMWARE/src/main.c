#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/usart.h>

#include "HAL.h"
#include "comm.h"
#include "neuron.h"
#include "izhi.h"

// #define DBG

#define BLINK_TIME			40
#define DATA_TIME			10
#define DEND_PING_TIME		200 // 1000 ms
#define	NID_PING_TIME		200 // 1000 ms
#define SEND_PING_TIME		80 // 80
#define BUTTON_PRESS_TIME	2
#define BUTTON_HOLD_TIME    100
#define LPUART_SETUP_TIME	100
#define CHANGE_NID_TIME 	200

typedef enum{
	DECAY 		=	0b0000,
    DEND1       =   0b0001,
    DEND2       =   0b0010,
    DEND3       =   0b0011,
	DEND4       =   0b0100,
	THRESHOLD 	=	0b0101,
	DEPRESSION	=	0b0111,
	LWINDOW		=	0b1000,
	DELAY		=	0b1101
} parameter_identifiers;

int main(void)
{
	uint8_t		i;

	// counters
	uint32_t	blink_time = 0; // counter for LED blink due to NID command
	uint16_t	data_time = 0; // counter for sending data to NID
	uint16_t	send_ping_time = 0; // counter for sending a downstream ping
	uint16_t	fire_delay_time = 0;
	int16_t 	depression_time = 0;
	uint8_t		fire_flag = 0;
	uint16_t	lpuart_setup_time = 0;
	uint8_t		change_nid_time = 0;
	int16_t		fire_data = 0; // hack to make sure a pulse sends peak neuron potential and hyperpolarization  data
	uint16_t	fire_delay_time_reset = FIRE_DELAY_TIME;
	fire_delay_time_reset = 40; // temporary
	uint16_t	fire_delay_overflow = 0; // used if firing period is less than delay time.

	// button debounce variables
	uint16_t	button_press_time = 0; 
	uint8_t		button_armed = 0;
	uint16_t	button_status = 0;

	uint8_t		decay_delay_time = 0;
	uint8_t		DECAY_DELAY_TIME = 3;

	int32_t		threshold_potential = MEMBRANE_THRESHOLD;
	message_t	message; // for constructing messages to send to the communications routine

	int32_t joegenta = 0;

	// initialize neuron
	neuron_t 	neuron;
	neuronInit(&neuron);

	// initialize izhi neuron
	ineuron_t spiky_i;
	RS_i(&spiky_i);

	for (i = 0; i < 100; i++) {
		step_i(&spiky_i, 0, 3);
	}

	neuron.dendrites[0].magnitude = 15000;
	neuron.dendrites[1].magnitude = 4300;
	neuron.dendrites[2].magnitude = 4300;
	neuron.dendrites[3].magnitude = 11000;

	neuron.dendrites[0].base_magnitude = 15000;
	neuron.dendrites[1].base_magnitude = 4300;
	neuron.dendrites[2].base_magnitude = 4300;
	neuron.dendrites[3].base_magnitude = 11000;

	// initialize communication buffers
	commInit();

	// setup hardware
	clock_setup();
	systick_setup(); // systick in microseconds
	gpio_setup();
	tim_setup();

	#ifdef DBG
	blink_flag = 1;
	#endif

	// main processing routine	
	for(;;)
	{
		if (main_tick == 1 && pause_flag == 0){
			// main tick every 5 ms
			main_tick = 0;
			// check to see if nid ping hasn't been received in last NID_PING_TIME ticks
			if (nid_ping_time > 0){
				nid_ping_time -= 1;
				if (nid_ping_time == 0){
					// nid no longer connected
					nid_distance = 100; // reset nid_keep_alive
					nid_pin = 0; // clear the nid pin
					nid_pin_out = 0;
					nid_i = NO_NID_I;
				}
			}

			if (change_nid_time++ > CHANGE_NID_TIME){
				change_nid_time = 0;
				closer_distance = nid_distance;
				closer_ping_count = 0;
			}
			
			// send a downstream ping every SEND_PING_TIME ticks
			if (send_ping_time++ > SEND_PING_TIME){
				// send downstream ping through axon
				addWrite(DOWNSTREAM_BUFF, downstream_ping_message);
				send_ping_time = 0;
			}

			if (lpuart_setup_time < LPUART_SETUP_TIME){
				lpuart_setup_time += 1;
			} else if (lpuart_setup_time == LPUART_SETUP_TIME){
				lpuart_setup_time += 1;
				#ifndef DBG
				lpuart_setup();
				#endif
			}

			/*
				Check to see if any comms flags have been set.
				Process any data that came from the NID.
			*/

			if (comms_flag != 0){
				switch (comms_flag){
					case DECAY:
						DECAY_DELAY_TIME = comms_data;
					case DEND1:
						spiky_i.a_inv = comms_data;
						break;
					case DEND2:
						spiky_i.b_inv = comms_data;
						break;
					case DEND3:
						spiky_i.c = comms_data;
						break;
					case DEND4:
						spiky_i.d = comms_data;
						break;
					case THRESHOLD:
						threshold_potential = comms_data;
					case DELAY:
						fire_delay_time_reset = comms_data;
					default:
						break;
				}
				comms_flag = 0;
				comms_data = 0;
			}

			/*
				nid_channel is the current channel, if any, that the NeuroByte is using to communicate
				with the NID. nid_channel should be cleared when NID tries to set a new NeuroByte to 
				identify_channel.

				The communication routine sets identify_time to zero when a new identify command is received.
			*/

			// check for clear channel command
			if (identify_time < IDENTIFY_TIME){
				if (identify_time == 0){
					if ((identify_channel == IDENTIFY_CLEAR) || (identify_channel == nid_channel)){
						// nid_channel = 0;
					}
				}
				identify_time += 1;
			}

			// check identify button
			button_status = gpio_get(PORT_IDENTIFY, PIN_IDENTIFY);
			// button_status = 0;
			nid_channel = 1;

			// if identify button is pressed and identify_time < IDENTIFY_TIME (i.e. NID sent 'identify'' message), set new nid_channel
			if (button_status == 0){
				// debounce
				button_press_time += 1;
				if (button_press_time >= BUTTON_HOLD_TIME){
					button_armed = 2;
					blink_flag = 1;
				} else if (button_press_time >= BUTTON_PRESS_TIME){
					button_armed = 1;
				}
			} else{
				// button not pressed
				if (button_armed == 0){
					button_press_time = 0;
				} else if (button_armed == 1 && nid_i != NO_NID_I){
					if (identify_channel != IDENTIFY_STOP) {
						if (nid_channel != identify_channel) {
							nid_channel = identify_channel;
							message.message = ((uint32_t) DATA_TYPE_MESSAGE | ((uint32_t) nid_channel << 20) | ((uint32_t) getFingerprint()));
							message.length = 32;
							addWrite(NID_BUFF, (const message_t) message);
						}
					}	
					identify_time = 1;
					button_armed = 0;
				} else if (button_armed == 2){
					if (neuron.learning_state == NONE){
						neuron.learning_state = HEBB;
					} else if (neuron.learning_state == HEBB){
						neuron.learning_state = NONE;
					}
					button_armed = 0;
				}
				button_press_time = 0;
			}
						
			/*
				Check dendrites for pings and adjut pins accordingly.
				Also check dendrites for pulses and calculate new membrane potential.
			*/
			checkDendrites(&neuron);
			
			// decay old dendrite contributions to the membrane potential
			if (++decay_delay_time >= DECAY_DELAY_TIME){
				decay_delay_time = 0;
				dendriteDecayStep(&neuron);
			}
			// decay the firing potential
			membraneDecayStep(&neuron);

			step_i(&spiky_i, 10 * spiky_i.scale, 3);

			// current membrane potential comes from dendrites and any left over firing potential
			neuron.potential = calcNeuronPotential(&neuron);
			neuron.potential += neuron.fire_potential;
			neuron.fire_potential += neuron.leaky_current;

			// send current membrane potential to NID if currently identified by NID
			if (nid_channel != 0){
				// send data every DATA_TIME ticks
				if (data_time++ > DATA_TIME){
					message.message = (((uint32_t) DATA_MESSAGE)) | ((uint16_t) ((int16_t)(spiky_i.potential / 8192)));						
					data_time = 0;
					message.length = 32;
					message.message |= (nid_channel << 21);
					addWrite(NID_BUFF,(const message_t) message);
				}
			}

			// if membrane potential is greater than threshold, fire
			if (neuron.potential > MEMBRANE_THRESHOLD){
				// fire for determined pulse width
				neuron.state = FIRE;
				neuron.fire_potential = HYPERPOLARIZATION;
				neuron.fire_time = PULSE_LENGTH;
				for (i=0; i<NUM_DENDS; i++){
					neuron.dendrites[i].current_value = 0;
					neuron.dendrites[i].state = OFF;
					if (neuron.learning_state == HEBB){
						calcDendriteWeightings(&neuron);
					}
				}
				if (neuron.learning_state == HEBB){
					calcDendriteWeightings(&neuron);
				}
				depression_time = 0;

				fire_data = neuron.potential; // hack to send fire data to NID

				// send downstream pulse
				if (fire_delay_time == 0){
					fire_delay_time = fire_delay_time_reset;
					fire_delay_overflow = 0;
				} else {
					fire_delay_overflow = fire_delay_time_reset - fire_delay_time;
				}
				fire_flag = 1;
			} else if (neuron.potential < HYPERPOLARIZATION){
				neuron.potential = HYPERPOLARIZATION;
			}

			if (fire_delay_time > 0){
				fire_delay_time -= 1;
			} else if (fire_flag == 1){
				if (fire_delay_overflow > 0)
					fire_delay_time = fire_delay_overflow; // TODO: make this an actual FIFO buffer of fire timings
				fire_flag = 0;
				addWrite(DOWNSTREAM_BUFF, pulse_message);
			}
			
			if (neuron.learning_state == HEBB){

				if (++depression_time >= DEPRESSION_TIME){
					for (i=0; i<NUM_DENDS; i++){
						neuron.dendrites[i].magnitude -= neuron.dendrites[i].base_magnitude;
						neuron.dendrites[i].magnitude *= 511;
						neuron.dendrites[i].magnitude /= 512;
						neuron.dendrites[i].magnitude += neuron.dendrites[i].base_magnitude;
					}
				}		
			}	
			joegenta = 0;
			for (i=0; i<NUM_DENDS; i++){
				joegenta += neuron.dendrites[i].magnitude - neuron.dendrites[i].base_magnitude;
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
					setLED(200,0,200);
					blink_time = 0;
				}
			} else if (neuron.state == FIRE){
				neuron.fire_time -= 1;
				if (neuron.fire_time == 0){
					neuron.state = INTEGRATE;
				}
				if (neuron.learning_state == HEBB){
					setLED(200,100,200);
				} else{
					LEDFullWhite();
				}
			} else if (neuron.state == INTEGRATE){
				if (neuron.learning_state == HEBB){
					if (neuron.potential > 5000){
						setLED((neuron.potential / 50), (200 - neuron.potential / 50) / 2, 0);
					} else{
						joegenta /= 8;
						if (joegenta / 80 > 240){
							setLED(180,0,180);
						} else if (joegenta > 0){
							setLED(40 + joegenta, 0, 40 + joegenta);
						} else if (joegenta < -10000){
							setLED(40,0, 40);
						} else if (joegenta < 0){
							setLED(40, 0, 40);
						} else{
							setLED(40,0,40);
						}
					}
				} else{
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
}
