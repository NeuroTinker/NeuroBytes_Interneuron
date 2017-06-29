#ifndef HAL_H_
#define HAL_H_

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/exti.h>

#include "pins.h"
#include "bit-bang.h"

#define DEBUG

#ifdef DEBUG
#include "debug.h"
#endif



/*
    Two clocks:
    -main_tick  : clocks the main processing routine at 5 ms
    -tick       : clocks the communication routine at 100 us
*/

extern volatile uint8_t main_tick;
extern uint8_t read_tick;
static const uint16_t gamma_lookup[1024];


void systick_setup(int xms);
void clock_setup(void);
void gpio_setup(void);
void tim_setup(void);
void LEDFullWhite(void);
void setLED(uint16_t r, uint16_t g, uint16_t b);
void writePinHigh(pin_t * pin);
void writePinLow(pin_t * pin);
void setPinAsOutput(pin_t * pin);


#endif
