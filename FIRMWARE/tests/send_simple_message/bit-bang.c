#include "bit-bang.h"

write_message_t * write_buffer[WRITE_BUFFER_SIZE];
uint8_t write_buffer_count = 0;
volatile active_input_t * active_inputs[NUM_INPUTS];

void initBitBang(void)
{
    uint8_t i;
    /*
        Initialize link between HAL->pins and bit-bang->pins
    */
    for (i=0; i<NUM_INPUTS; i++){
        input_pins[i]->active_input = active_inputs[i];
        active_inputs[i]->pin = input_pins[i];
    }
}

void addMessage(uint64_t new_message, uint8_t num_bits_to_write, pin_t * output_pins[NUM_OUTPUTS], uint8_t num_output_pins)
{
    /*
        Add a new message to the write buffer (FIFO)
    */
    uint8_t i;
    write_message_t * new_write_message;

    if (write_buffer_count < WRITE_BUFFER_SIZE){ // check for buffer overflow
        new_write_message = write_buffer[write_buffer_count];
        write_buffer_count += 1; 
    } else{
        #ifdef DEBUG
        printf("write_buffer overflow");
        #endif
        return;
    }

    new_write_message->message = new_message;
    new_write_message->bits_left_to_write = num_bits_to_write;
    memcpy(new_write_message->output_pins, output_pins, num_output_pins * sizeof(pin_t));
    /*
    for(i=0; i<num_output_pins; i++){
        new_write_message->output_pins[i] = output_pins[i];
    }
    */
    new_write_message->num_output_pins = num_output_pins;
}

void writeBit(void)
{
    /*
        Write a bit from the bottom message of write_buffer to the desired output pins
    */

    uint8_t i;
    uint32_t value;

    write_message_t * write_message;

    if (write_buffer_count > 0){

        write_message = write_buffer[0];

        value = write_message->message | write_message->bits_left_to_write;

        for(i=0; i<write_message->num_output_pins; i++){            
            if (value != 0){
                writePinHigh(write_message->output_pins[i]);
            } else{
                writePinLow(write_message->output_pins[i]);
            }
        }

        if (write_message->bits_left_to_write >= 1){
            write_message->bits_left_to_write -= 1;
        } else{
            for (i=0; i<=write_buffer_count; i++){
                write_buffer[i] = write_buffer[i+1];
            }
            write_buffer_count -= 1;
        }
    }
}

void readBit(pin_t * pin)
{
    uint8_t i;
    bool value;
    uint8_t keep_reading;

    /*
        Read one bit
    */
    active_inputs[i]->message <<= 1;
    active_inputs[i]->message |= readPin(pin);
    active_inputs[i]->num_bits_to_read -= 1;

    /*
        Process message if there are no more bits to read
    */

    if (active_inputs[i]->num_bits_to_read == 0){
        keep_reading = processMessage(pin, pin->input_buffer->message);
        // if there is a data packet coming, keep reading.
        if (keep_reading == 0){
            // no data packet coming, reset the input pin
            pin->active = False;
            resetInputPin(pin);
        } else{
            // data packet is coming so keep reading
            active_inputs[i]->num_bits_to_read += keep_reading;
            active_inputs[i]->state = DATA;
        }
    } 
}

