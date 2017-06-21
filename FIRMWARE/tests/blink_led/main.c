#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/exti.h>

#define LED_GREEN_PIN GPIO1
#define LED_GREEN_PORT GPIOA
#define LED_RED_PIN GPIO7
#define LED_RED_PORT GPIOB

int main()
{
    /* Enable GPIOB clock. */
	rcc_periph_clock_enable(RCC_GPIOA);
	rcc_periph_clock_enable(RCC_GPIOB);

	/* set pins to output mode, push pull */
	gpio_mode_setup(LED_RED_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_RED_PIN);
	gpio_mode_setup(LED_GREEN_PORT, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_GREEN_PIN);
    uint8_t i;

    for(;;){
		/* toggle each led in turn */
		gpio_toggle(LED_GREEN_PORT, LED_GREEN_PIN);
		for (i = 0; i < 100000; i++) {	/* Wait a bit. */
			__asm__("nop");
		}
		gpio_toggle(LED_RED_PORT, LED_RED_PIN);
		for (i = 0; i < 100000; i++) {	/* Wait a bit. */
			__asm__("nop");
		}
    }
}