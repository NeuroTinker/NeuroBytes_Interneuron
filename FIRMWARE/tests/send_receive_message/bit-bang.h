#ifndef BIT_BANG_H
#define BIT_BANG_H

#include <string.h>

#include "pins.h"
#include "HAL.h"

#define WRITE_BUFFER_SIZE 10

typedef struct {
    uint64_t message;
    uint8_t num_bits_left_to_write;
    pin_group_name_t output_pin_group;
} output_message_t;

static output_message_t * write_buffer[WRITE_BUFFER_SIZE];
static uint8_t write_buffer_count;

void addMessage(uint64_t message_binary, uint8_t num_bits_to_write, pin_group_name_t output_pin_group);
void readBit(pin_t * pin); // read an active input
void writeBit(pin_group_t * output); // write from the write_buffer

/*
Eventually implement port-wide reads/writes so that io is more synchronous and efficient
*/

#endif