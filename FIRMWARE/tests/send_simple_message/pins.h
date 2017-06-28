#ifndef PINS_H
#define PINS_H
#include "pin-map.h"

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

struct input_buffer_t{
    uint64_t message;
    uint8_t read_tick;
    uint8_t num_bits_to_read;
    input_state_t state;
};

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
    pin_t * pins,
    uint8_t num_pins
} pin_group_t;

static pin_t * all_pins[NUM_PINS];
static pin_group_t * output_pins;
static pin_group_t * input_pins;
static pin_t * nid_pin;

static const pin_t * axon_output_pins[NUM_AXONS];

void initPins(void);
static pin_address_t * initPinAddress(uint32_t port, uint32_t pin);
static input_buffer_t * initInputBuffer(void);

static void freeInputBuffer(pin_t * pin);

void startRead(pin_t * pin, uint8_t read_tick);
void endRead(pin_t * pin);
pin_group_t getInputPins(void);

void selectOutputPins(pin_group_name_t output_group_name);
pin_group_t getOutputPins(void);

#endif



