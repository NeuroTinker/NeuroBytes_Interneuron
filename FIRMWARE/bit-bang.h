#ifndef BIT_BANG_H
#define BIT_BANG_H

#include "HAL.h"
#include "comm.h"



void readBit(void); // read the active inputs
void writeBit(void); // write from the write_buffer
void startRead(pin_t *pin, uint8_t num_bits_to_read); // start reading from an input pin
void endRead(pin_t *pin); // stop reading from an input pin

#endif