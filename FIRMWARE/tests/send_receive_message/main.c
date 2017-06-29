#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/exti.h>

#define DEBUG

#ifdef DEBUG
#include "debug.h"
#endif

#include "pins.h"
#include "HAL.h"
#include "bit-bang.h"

int main(void)
{
	uint64_t new_message = 0b11111111;
	uint8_t num_bits = 8;
	pin_group_name_t output = AXONS;
	initPins();
	// initialize hardware
	clock_setup();
	systick_setup(100); // systick in microseconds
	gpio_setup();
	tim_setup();
	setLED(200,200,200);
	//output_pins[0] = all_pins[1];
	for(;;)
	{
		if (main_tick == 1){
			// main tick every 5 ms
			main_tick = 0;
			addMessage(new_message, num_bits, output);
		}
	}
}
