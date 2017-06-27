#ifndef BIT_BANG_H
#define BIT_BANG_H

#include <string.h>

#include "HAL.h"

#define WRITE_BUFFER_SIZE 10


void initBitBang(void);
void startRead(pin_t * pin, uint8_t bits_to_read);
void endRead(pin_t * pin);
void addMessage(uint64_t new_message, uint8_t num_bits_to_write, pin_t * output_pins[], uint8_t num_output_pins);
void readBit(uint8_t read_tick); // read the active inputs
void writeBit(void); // write from the write_buffer

#endif