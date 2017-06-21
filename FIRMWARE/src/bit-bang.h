#ifndef BIT_BANG_H
#define BIT_BANG_H

#include "HAL.h"
#include "comm.h"

#define WRITE_BUFFER_SIZE 10

typedef struct {
    uint64_t message,
    uint8_t bits_left_to_write,
    pin_t * output_pins[],
    uint8_t num_output_pins
} write_message_t;

typedef enum{
    OFF,
    MESSAGE,
    DATA
} input_state_t;

typedef struct {
    uint64_t message,
    pin_t pin,
    uint8_t read_tick,
    uint8_t num_bits_to_read,
    input_state_t state
} active_input_t;

extern volatile active_input_t * active_inputs[NUM_INPUTS];

void readBit(uint8_t read_tick); // read the active inputs
void writeBit(void); // write from the write_buffer
void startRead(pin_t *pin, uint8_t num_bits_to_read); // start reading from an input pin
void endRead(pin_t *pin); // stop reading from an input pin

#endif