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
#define SEND_PING_TIME		80
#define BUTTON_PRESS_TIME	2

int main(void)
{
	uint32_t	blink_time = 0;
	uint32_t	wait_time = 0;
	uint16_t	data_time = 0;
	uint16_t	message_data = 0;
	uint16_t	send_ping_time = 0;
	uint16_t	button_press_time = 0;
	uint8_t		button_armed = 0;
	uint16_t		button_status = 0;
	uint32_t	nid_channel = 0b000;
	uint32_t	message = 0;

	neuron_t 	neuron;
	uint8_t		i;
	neuronInit(&neuron);
	commInit();

	clock_setup();
	systick_setup(100);
	gpio_setup();
	tim_setup();
	gpio_setup();
	exti_select_source(EXTI3, GPIOB);
	setLED(200,0,0);
	//systick_setup(100000);

	//	MMIO32(SYSCFG_BASE + 0x0c) = 0b1111 << 12;

	
	for(;;)
	{
		//if (exti_get_flag_status(PIN_DEND5_EX) != 0) setLED(0,200,200);
		//gpio_set(PORT_AXON_OUT, PIN_AXON_OUT);
		//if (gpio_get(PORT_DEND5_EX, PIN_DEND5_EX) != 0) setLED(0,200,200);
		//setLED(0,0,10);
		if (main_tick == 1){
			// 5 ms
			//setLED(200,0,200);
			main_tick = 0;
			//gpio_toggle(PORT_AXON_OUT, PIN_AXON_OUT);
			//downstream_write_buffer = 0b01010101010101010101010101010101;
			//downstream_write_buffer_ready = 1;
			/*
			if (++wait_time == 10){
				downstream_write_buffer = BLINK_MESSAGE;
				downstream_write_buffer_ready = 1;
				wait_time = 0;
			}
			*/
			
			/*
			if (++wait_time == DATA_TIME){
				wait_time = 0;
				message_data = MMIO32(SYSCFG_BASE + 0x08);
				nid_write_buffer = DATA_MESSAGE | message_data;
				nid_write_buffer_ready = 1;
			}
			*/
			/*
			if (++wait_time == 50){
				wait_time = 0;
				downstream_write_buffer = PULSE_MESSAGE;
				downstream_write_buffer_ready = 1;
			}
			*/
			
			
			if (nid_ping_time == 0){
				nid_keep_alive = NID_PING_KEEP_ALIVE;
				nid_pin = 0;
				nid_pin_out = 0;
			}else {
				nid_ping_time -= 1;
			}
			
			if (send_ping_time++ > SEND_PING_TIME){
				addWrite(DOWNSTREAM_BUFF, DEND_PING);
				//downstream_write_buffer = DEND_PING;
				//downstream_write_buffer_ready = 1;
				send_ping_time = 0;
			}

			button_status = gpio_get(PORT_IDENTIFY, PIN_IDENTIFY);
			button_status >>= 8;
			button_status &= 0b1;
			if (identify_time > 0){
				identify_time -= 1;
				if (identify_channel == 0){
					// clear all channels
					nid_channel = 0;
				}
			}
			if (button_status == 0){ // !=
				//setLED(200,200,200);
				if (button_press_time++ >= BUTTON_PRESS_TIME){
					button_armed = 1;
					button_press_time = 0;
				}
			} else if (button_armed == 1){
				if (identify_time > 0){
					nid_channel = identify_channel;
				} else{
					neuron.fire_potential += 110;
				}
				button_armed = 0;
			} else{
				button_press_time = 0;
			}
			
			if (nid_channel != 0){
				if (data_time++ > DATA_TIME){
					data_time = 0;
					//message = DATA_MESSAGE | (nid_channel << 19) | (uint16_t) neuron.potential | (nid_keep_alive << 22);
					message = DATA_MESSAGE | (uint16_t) neuron.potential | (nid_channel << 19) | (nid_keep_alive << 22);
					addWrite(NID_BUFF,message);
				}
			}
			
			checkDendrites(&neuron);
			
			dendriteDecayStep(&neuron);
			membraneDecayStep(&neuron);

			neuron.potential = calcNeuronPotential(&neuron);
			neuron.potential += neuron.fire_potential;
			// debug breathe led code
			/*
			neuron.potential += 1;
			if (neuron.potential == 200) neuron.potential = 0;
			*/

			if (neuron.potential > MEMBRANE_THRESHOLD){
				neuron.state = FIRE;
				neuron.fire_potential = HYPERPOLARIZATION;
				neuron.fire_time = PULSE_LENGTH;
				for (i=0; i<DENDRITE_COUNT; i++){
					neuron.dendrites[i].current_value = 0;
					neuron.dendrites[i].state = OFF;
				}
				addWrite(DOWNSTREAM_BUFF, PULSE_MESSAGE);
			}

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
				if (neuron.potential > 140){
					setLED(200,0,0);
				} else if (neuron.potential > 0){
					setLED(neuron.potential * 10 / 7, 200 - (neuron.potential * 10 / 7), 0);
				} else if (neuron.potential < -140){
					setLED(0,0, 200);
				} else if (neuron.potential < 0){
					setLED(0, 200 + (neuron.potential * 10 / 7), -1 * neuron.potential * 10 / 7);
				} else{
					setLED(0,200,0);
				}
			}
		}
	}
}
