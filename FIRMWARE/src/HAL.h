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

typedef struct {
    uint32_t pin,
    uint32_t port
} pin_address_t;

typedef enum {
    INHIB,
    EXCIT
} pin_type_t;

typedef enum {
    DEND,
    AXON
} port_type_t;

typedef enum {
    INPUT,
    OUTPUT
} pin_state_t;

typedef struct pin_t pin_t;

struct pin_t{
    pin_address_t address,
    pin_type_t pin_type,
    port_type_t port_type,
    pin_t * complimentary_pin_ptr,
    pin_state_t state,
    uint32_t exti,
    dendrite_t * dendrite_ptr,
    uint8_t index
};

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
<<<<<<< HEAD:FIRMWARE/src/HAL.h
static const uint16_t gamma_lookup[1024];

=======
extern volatile uint8_t tick;
extern volatile uint8_t read_tick;
static const uint16_t gamma_lookup[1024];
static const uint16_t device_id[4];
>>>>>>> development:FIRMWARE/src/HAL.h

void systick_setup(int xms);
void clock_setup(void);
void gpio_setup(void);
void tim_setup(void);
void LEDFullWhite(void);
void setLED(uint16_t r, uint16_t g, uint16_t b);
void setAsInput(uint32_t port, uint32_t pin);
void setAsOutput(uint32_t port, uint32_t pin);
uint8_t getFingerprint(void);


#endif
