#ifndef HAL_H_
#define HAL_H_

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/exti.h>
#include <libopencm3/stm32/usart.h>

#include "comm.h"

#define NUM_INPUTS 11
#define HAS_AXONS   true
#define HAS_DENDS   true
#define NUM_AXONS   3
#define NUM_DENDS   4
#define COMPLIMENTARY_I(i)  i + (i % 2) - ((i+1) % 2)

/*
    Define all pins
*/

#define LPUART1         LPUART1_BASE

#define PORT_LPUART1_TX GPIOA
#define PORT_LPUART1_RX GPIOA

#define PIN_LPUART1_RX  GPIO13
#define PIN_LPUART1_TX  GPIO14

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

#define PORT_DEND1_IN   GPIOB
#define PORT_DEND1_EX   GPIOB
#define PORT_DEND2_IN   GPIOA
#define PORT_DEND2_EX   GPIOA
#define PORT_DEND3_IN   GPIOA
#define PORT_DEND3_EX   GPIOA
#define PORT_DEND4_IN   GPIOA
#define PORT_DEND4_EX   GPIOA

#define PIN_AXON1_EX    GPIO8
#define PIN_AXON1_IN    GPIO15
#define PIN_AXON2_EX    GPIO4
#define PIN_AXON2_IN    GPIO9
#define PIN_AXON3_EX    GPIO14
#define PIN_AXON3_IN    GPIO10

#define PIN_DEND1_IN    GPIO1
#define PIN_DEND1_EX    GPIO0
#define PIN_DEND2_IN    GPIO7
#define PIN_DEND2_EX    GPIO6
#define PIN_DEND3_IN    GPIO5
#define PIN_DEND3_EX    GPIO4
#define PIN_DEND4_IN    GPIO3
#define PIN_DEND4_EX    GPIO2

#define ACTIVATE_INPUT(I, PIN)   active_input_pins[(I)] = PIN; active_input_tick[(I)] = (read_tick + 2) % 3

extern const uint16_t complimentary_pins[NUM_INPUTS];
extern const uint32_t complimentary_ports[NUM_INPUTS];
extern volatile uint16_t active_input_pins[NUM_INPUTS];
extern uint32_t active_input_ports[NUM_INPUTS];
extern volatile uint16_t active_output_pins[NUM_INPUTS];
extern uint32_t active_output_ports[NUM_INPUTS];
extern volatile uint8_t active_input_tick[NUM_INPUTS];

extern volatile uint8_t dendrite_pulse_flag[NUM_INPUTS];
extern volatile uint8_t dendrite_ping_flag[NUM_INPUTS];

/*
    Two clocks:
    -main_tick  : clocks the main processing routine at 5 ms
    -tick       : clocks the communication routine at 100 us
*/

typedef struct{
    uint8_t device_type;
    uint32_t unique_id;
    uint8_t firmware_version;
} fingerprint_t;

extern volatile uint8_t main_tick;
extern volatile uint8_t tick;
static const uint16_t gamma_lookup[1024];
static const uint16_t device_id[4];

void systick_setup(void);
void clock_setup(void);
void gpio_setup(void);
void tim_setup(void);
void lpuart_setup(void);
void LEDFullWhite(void);
void setLED(uint16_t r, uint16_t g, uint16_t b);
void setAsInput(uint32_t port, uint32_t pin);
void setAsOutput(uint32_t port, uint32_t pin);
const uint16_t * getFingerprint(void);


#endif
