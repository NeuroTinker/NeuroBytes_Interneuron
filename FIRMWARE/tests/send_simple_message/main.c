#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/exti.h>

#include "bit-bang.h"
#include "HAL.h"
#include "bit-bang.h"

#define DEBUG

#ifdef DEBUG
#include "debug.h"
#endif

int main(void)
{
	uint8_t		i;
	break0;
	uint64_t new_message = 0b11111111;
	uint8_t num_bits = 8;
	pin_t * output_pins[NUM_OUTPUTS];
	uint8_t num_outputs = 1;
	// initialize hardware
	clock_setup();
	systick_setup(100); // systick in microseconds
	gpio_setup();
	tim_setup();
	pinInit();
	setLED(200,200,200);
	break1;
	output_pins[0] = all_pins[1];
	for(;;)
	{
		if (main_tick == 1){
			// main tick every 5 ms
			main_tick = 0;
			addMessage(new_message, num_bits, output_pins, num_outputs);
		}
	}
}
