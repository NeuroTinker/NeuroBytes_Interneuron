#include "bit-bang.h"

static output_message_t * write_buffer[WRITE_BUFFER_SIZE];
static uint8_t write_buffer_count = 0;
volatile active_input_t * active_inputs[NUM_INPUTS];

void addMessage(uint64_t message_binary, uint8_t num_bits_to_write, pin_group_name_t output_pin_group)
{
    /*
        Add a new message to the write buffer (FIFO)
    */
    uint8_t i;
    output_message_t new_message;
    new_message.message = message_binary;
    new_message.num_bits_left_to_write = num_bits_to_write;
    new_message.output_pin_group = output_pin_group;

    // not using dynamic memory for write buffer because memory could be corrupted during interrupt
    if (write_buffer_count < WRITE_BUFFER_SIZE){ // check for buffer overflow
        memcpy(write_buffer[write_buffer_count], new_message, sizeof(output_message_t));
        write_buffer_count += 1; 
    } else{
        #ifdef DEBUG
        printf("write_buffer overflow");
        #endif
        return;
    }
}

void readBit(pin_t * pin)
{
    uint8_t i;
    uint8_t keep_reading;
    input_buffer_t * input_buffer = pin->input_buffer;

    /*
        Read one bit
    */
    input_buffer->message <<= 1;
    input_buffer->message |= readPin(pin);
    input_buffer->num_bits_to_read -= 1;

    /*
        Process message if there are no more bits to read
    */

    if (input_buffer->num_bits_to_read == 0){
        keep_reading = processMessage(pin, input_buffer->message);
        // if there is a data packet coming, keep reading.
        if (keep_reading == 0){
            // no data packet coming, reset the input pin
            pin->active = False;
            resetInputPin(pin);
        } else{
            // data packet is coming so keep reading
            input_buffer->num_bits_to_read += keep_reading;
            input_buffer->state = DATA;
        }
    } 
}

void writeBit(pin_group_t output)
{
    /*
        Write a bit from the bottom message of write_buffer to the desired output pins
    */

    uint8_t i;
    bool value;
    pin_t num_output_pins = output->num_output_pins;
    output_message_t * write_message = write_buffer[0];

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


