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

#define PORT_AXON_IN    GPIOA
#define PORT_AXON_OUT   GPIOA
#define PORT_DEND1_EX   GPIOB
#define PORT_DEND1_IN   GPIOB
#define PORT_DEND2_EX   GPIOA
#define PORT_DEND2_IN   GPIOA
#define PORT_DEND3_EX   GPIOA
#define PORT_DEND3_IN   GPIOC
#define PORT_DEND4_EX   GPIOB
#define PORT_DEND4_IN   GPIOB
#define PORT_DEND5_EX   GPIOB
#define PORT_DEND5_IN   GPIOB

#define PIN_AXON_IN     GPIO9
#define PIN_AXON_OUT    GPIO10
#define PIN_DEND1_EX    GPIO1
#define PIN_DEND1_IN    GPIO0
#define PIN_DEND2_EX    GPIO7
#define PIN_DEND2_IN    GPIO6
#define PIN_DEND3_EX    GPIO1
#define PIN_DEND3_IN    GPIO14
#define PIN_DEND4_EX    GPIO7
#define PIN_DEND4_IN    GPIO6
#define PIN_DEND5_EX    GPIO4
#define PIN_DEND5_IN    GPIO3


extern volatile uint8_t main_tick;
extern volatile uint8_t tick;
static const uint16_t gamma_lookup[1024];


// uint8_t     read_time = 0;


void systick_setup(int xms);
void clock_setup(void);
void gpio_setup(void);
void tim_setup(void);
void LEDFullWhite(void);
void setLED(uint16_t r, uint16_t g, uint16_t b);
void setAsInput(uint32_t port, uint32_t pin);
void setAsOutput(uint32_t port, uint32_t pin);


//void tim2_isr(void);

//extern volatile unsigned char ms_tick;

#endif
