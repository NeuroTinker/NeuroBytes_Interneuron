#ifndef BIT_BANG_H
#define BIT_BANG_H

#include <string.h>

#include "pins.h"
#include "HAL.h"

#define WRITE_BUFFER_SIZE 6

typedef struct {
    uint64_t message;
    uint8_t num_bits_left_to_write;
    pin_group_name_t output_pin_group;
} output_message_t;

void addMessage(uint64_t message_binary, uint8_t num_bits_to_write, pin_group_name_t output_pin_group);
void writeBit(pin_group_t * output); // write from the write_buffer

#endif