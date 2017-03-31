#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/exti.h>

#include "HAL.h"
#include "comm.h"

int main(void)
{
	//clock_setup();
	initializeVars();
	gpio_setup();
	tim_setup();
	setLED(200,200,200);
	//systick_setup(100000);
	
	for(;;)
	{
		if (main_tick == 1){
			main_tick = 0;

			downstream_write_buffer = 0b01010101010101010101010101010101;
			downstream_write_buffer_ready += 1;
		}
	}
}
