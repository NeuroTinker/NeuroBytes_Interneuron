#ifndef HAL_H_
#define HAL_H_

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/exti.h>

//#include "bit-bang.h"
/*
    Define all pins
*/

#define PORT_R_LED      GPIOB
#define PORT_G_LED      GPIOA
#define PORT_B_LED      GPIOB
#define PIN_R_LED		GPIO7
#define PIN_G_LED		GPIO1
#define PIN_B_LED		GPIO6

#define PORT_AXON1_EX   GPIOA
#define PORT_AXON1_IN   GPIOC

#define PORT_DEND1_EX   GPIOB
#define PORT_DEND1_IN   GPIOB

#define PIN_AXON1_EX    GPIO8
#define PIN_AXON1_IN    GPIO15

#define PIN_DEND1_EX    GPIO1
#define PIN_DEND1_IN    GPIO0

#define DEND1_EX        all_pins[2]
#define DEND1_IN        all_pins[3]
#define AXON1_EX        all_pins[0]
#define AXON1_IN        all_pins[1]

#define NUM_INPUTS      3
#define NUM_PINS        4
#define NUM_OUTPUTS     3

typedef struct {
    uint32_t pin;
    uint32_t port;
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

typedef struct active_input_t active_input_t;
typedef struct pin_t pin_t;

struct pin_t{
    pin_address_t * address;
    pin_type_t pin_type;
    port_type_t port_type;
    pin_t * complimentary_pin_ptr;
    pin_state_t state;
    uint32_t exti;
    //dendrite_t * dendrite_ptr;
    active_input_t * active_input;
};

typedef struct {
    uint64_t message;
    uint8_t bits_left_to_write;
    uint8_t num_output_pins;
    pin_t * output_pins[NUM_OUTPUTS];
} write_message_t;

typedef enum{
    OFF,
    MESSAGE,
    DATA
} input_state_t;

struct active_input_t{
    uint64_t message;
    pin_t * pin;
    uint8_t read_tick;
    uint8_t num_bits_to_read;
    input_state_t state;
};
extern pin_t * all_pins[NUM_PINS];

/*
    Two clocks:
    -main_tick  : clocks the main processing routine at 5 ms
    -tick       : clocks the communication routine at 100 us
*/

extern volatile uint8_t main_tick;
static const uint16_t gamma_lookup[1024];


void systick_setup(int xms);
void clock_setup(void);
void gpio_setup(void);
void tim_setup(void);
void LEDFullWhite(void);
void setLED(uint16_t r, uint16_t g, uint16_t b);
void pinInit(void);
void startRead(pin_t * pin, uint8_t bits_to_read);
void endRead(pin_t * pin);
void writePinHigh(pin_t * pin);
void writePinLow(pin_t * pin);
bool readPin(pin_t * pin);
void resetPinInterrupt(pin_t * pin);
void setPinAsInput(pin_t * pin);
void setPinAsOutput(pin_t * pin);


#endif
