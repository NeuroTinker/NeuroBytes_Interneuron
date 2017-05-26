#ifndef HAL_H_
#define HAL_H_

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/exti.h>

#include "comm.h"

/*
    Define all pins
*/

#define PORT_R_LED      GPIOB
#define PORT_G_LED      GPIOA
#define PORT_B_LED      GPIOB
#define PIN_R_LED		GPIO7
#define PIN_G_LED		GPIO1
#define PIN_B_LED		GPIO6

#define PORT_IDENTIFY	GPIOB
#define PIN_IDENTIFY	GPIO3

#define PORT_AXON1_EX   GPIOA
#define PORT_AXON1_IN   GPIOC
#define PORT_AXON2_EX   GPIOB
#define PORT_AXON2_IN   GPIOA
#define PORT_AXON3_EX   GPIOC
#define PORT_AXON3_IN   GPIOA

#define PORT_DEND1_EX   GPIOB
#define PORT_DEND1_IN   GPIOB
#define PORT_DEND2_EX   GPIOA
#define PORT_DEND2_IN   GPIOA
#define PORT_DEND3_EX   GPIOA
#define PORT_DEND3_IN   GPIOA
#define PORT_DEND4_EX   GPIOA
#define PORT_DEND4_IN   GPIOA

#define PIN_AXON1_EX    GPIO8
#define PIN_AXON1_IN    GPIO15
#define PIN_AXON2_EX    GPIO4
#define PIN_AXON2_IN    GPIO9
#define PIN_AXON3_EX    GPIO14
#define PIN_AXON3_IN    GPIO10

#define PIN_DEND1_EX    GPIO1
#define PIN_DEND1_IN    GPIO0
#define PIN_DEND2_EX    GPIO7
#define PIN_DEND2_IN    GPIO6
#define PIN_DEND3_EX    GPIO5
#define PIN_DEND3_IN    GPIO4
#define PIN_DEND4_EX    GPIO3
#define PIN_DEND4_IN    GPIO2

/*
    Two clocks:
    -main_tick  : clocks the main processing routine at 5 ms
    -tick       : clocks the communication routine at 100 us
*/

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
