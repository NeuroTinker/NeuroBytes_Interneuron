#include "bit-bang.h"

//static output_message_t * write_buffer[WRITE_BUFFER_SIZE];
//static uint8_t write_buffer_count = 0;

void addMessage(uint64_t message_binary, uint8_t num_bits_to_write, pin_group_name_t output_pin_group)
{
    /*
        Add a new message to the write buffer (FIFO)
    */
    output_message_t new_message;
    new_message.message = message_binary;
    new_message.num_bits_left_to_write = num_bits_to_write;
    new_message.output_pin_group = output_pin_group;

    // not using dynamic memory for write buffer because memory could be corrupted during interrupt
    if (write_buffer_count < WRITE_BUFFER_SIZE){ // check for buffer overflow
        memcpy(write_buffer[write_buffer_count], &new_message, sizeof(output_message_t));
        write_buffer_count += 1; 
    } else{
        #ifdef DEBUG
        printf("write_buffer overflow");
        #endif
        return;
    }

    if (write_buffer_count == 1)
        selectOutputPins(output_pin_group);
}

void writeBit(pin_group_t * output)
{
    /*
        Write a bit from the bottom message of write_buffer to the desired output pins
    */

    uint8_t i;
    bool value;
    uint8_t num_output_pins = output->num_pins;
    output_message_t * write_message = write_buffer[0];

    if (write_buffer_count > 0){

        write_message = write_buffer[0];

        value = write_message->message | ( 1 << (write_message->num_bits_left_to_write - 1));

        for(i=0; i<num_output_pins; i++){            
            if (value != 0){
                writePinHigh(output->pins[i]);
            } else{
                writePinLow(output->pins[i]);
            }
        }

        if (write_message->num_bits_left_to_write >= 1){
            write_message->num_bits_left_to_write -= 1;
        } else{
            for (i=0; i<=write_buffer_count; i++){
                write_buffer[i] = write_buffer[i+1];
            }
            selectOutputPins(write_buffer[0]->output_pin_group);
            write_buffer_count -= 1;
        }
    }
}


