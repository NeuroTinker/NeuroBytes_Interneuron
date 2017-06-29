#ifndef PINS_H
#define PINS_H
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/exti.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "pin-map.h"

typedef struct pin_t pin_t;

typedef struct {
    uint32_t port;
    uint32_t pin;
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

typedef enum{
    MESSAGE,
    DATA
} input_state_t;

typedef struct {
    uint64_t message;
    uint8_t read_tick;
    uint8_t num_bits_to_read;
    input_state_t state;
} input_buffer_t;

struct pin_t{
    pin_address_t * address;
    pin_type_t pin_type;
    port_type_t port_type;
    pin_t * complimentary_pin_ptr;
    pin_state_t io_state;
    uint32_t exti;
    bool active;
    //dendrite_t * dendrite_ptr;
    input_buffer_t * input_buffer;
};

typedef enum{
    ALL,
    NID,
    AXONS,
    NONE
} pin_group_name_t;

typedef struct {
    pin_t ** pins;
    uint8_t num_pins;
} pin_group_t;


void initPins(void);
pin_address_t * initPinAddress(uint32_t port, uint32_t pin);

void selectOutputPins(pin_group_name_t output_group_name);
pin_group_t * getOutputPins(void);

#endif



