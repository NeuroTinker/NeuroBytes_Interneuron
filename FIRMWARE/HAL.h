#ifndef HAL_H_
#define HAL_H_

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/exti.h>

#include "comm.h"

#define PORT_LED		GPIOA
#define PIN_R_LED		GPIO2
#define PIN_G_LED		GPIO5
#define PIN_B_LED		GPIO3

#define PORT_IDENTIFY	GPIOA
#define PIN_IDENTIFY	GPIO8


extern volatile uint8_t main_tick;
extern volatile uint8_t tick;
static const uint16_t gamma_lookup[1024];

// uint8_t     read_time = 0;

void systick_setup(int freq);
void clock_setup(void);
void gpio_setup(void);
void tim_setup(void);
void LEDfullWhite(void);
void setLED(uint16_t r, uint16_t g, uint16_t b);
void setAsInput(gpio_port port, gpio_pin pin);
void setAsOutput(gpio_port port, gpio_pin pin);

void exti0_isr(void);
void exti1_isr(void);
void exti3_isr(void);
void exti4_isr(void);
void exti6_isr(void);
void exti7_isr(void);

//extern volatile unsigned char ms_tick;

#endif
