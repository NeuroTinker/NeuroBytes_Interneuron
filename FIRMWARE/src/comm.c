
#include "comm.h"
#include "HAL.h"

write_buffer_t write_buffer;
read_buffer_t read_buffer[11] = {
    [0 ... 10] = { .message=0, .bits_left_to_read=5, .callback=processMessageHeader}
};
//uint32_t read_buffer[11] = {0,0,0,0,0,0,0,0,0,0,0};
//uint8_t read_buffer_bits_left[11] = 0,0,0,0,0,0,0,0,0,0,0};
//void (*read_buffer_callback[11]) (uint32_t);

volatile uint16_t active_input_pins[11] = {0,0,0,0,0,0,0,0,0,0,0};

volatile uint8_t active_input_tick[11] = {0,0,0,0,0,0,0,0,0,0,0};

volatile uint16_t active_output_pins[11] = {PIN_AXON1_IN, PIN_AXON2_IN,PIN_AXON3_EX,0,0,0,0,0,0,0,0};

volatile uint32_t dendrite_pulses[4] = {0,0,0,0};
volatile uint8_t dendrite_pulse_count = 0;

volatile uint8_t blink_flag = 0;

volatile uint32_t nid_ping_time = 0;

volatile uint16_t nid_pin = 0;
uint32_t nid_port = 0;
volatile uint16_t nid_pin_out = 0;
uint32_t nid_port_out = 0;
uint8_t nid_i      =    4;
volatile uint32_t  nid_keep_alive = NID_PING_KEEP_ALIVE;


/* 
All available input pins are
    = {
        PIN_AXON1_IN,
        PIN_AXON2_IN,
        PIN_DEND1_EX,
        PIN_DEND1_IN, 
        PIN_DEND2_EX,
        PIN_DEND2_IN,
        PIN_DEND3_EX,
        PIN_DEND3_IN,
        PIN_DEND4_EX,
        PIN_DEND4_IN
    };
*/

uint32_t active_input_ports[11] = {
    PORT_AXON1_IN,
    PORT_AXON2_IN,
    PORT_AXON3_IN,
    PORT_DEND1_EX,
    PORT_DEND1_IN,
    PORT_DEND2_EX,
    PORT_DEND2_IN,
    PORT_DEND3_EX,
    PORT_DEND3_IN,
    PORT_DEND4_EX,
    PORT_DEND4_IN
};

uint32_t active_output_ports[11] = {
    PORT_AXON1_EX,
    PORT_AXON2_EX,
    PORT_AXON3_EX,
    PORT_DEND1_EX,
    PORT_DEND1_IN,
    PORT_DEND2_EX,
    PORT_DEND2_IN,
    PORT_DEND3_EX,
    PORT_DEND3_IN,
    PORT_DEND4_EX,
    PORT_DEND4_IN
};

uint16_t complimentary_pins[11] = {
    PIN_AXON1_EX,
    PIN_AXON2_EX,
    PIN_AXON3_EX,
    PIN_DEND1_IN,
    PIN_DEND1_EX,
    PIN_DEND2_IN,
    PIN_DEND2_EX,
    PIN_DEND3_IN,
    PIN_DEND3_EX,
    PIN_DEND4_IN,
    PIN_DEND4_EX
};

volatile uint8_t dendrite_pulse_flag[11] = {0,0,0,0,0,0,0,0,0,0,0};
volatile uint8_t dendrite_ping_flag[11] = {0,0,0,0,0,0,0,0,0,0,0};
uint8_t write_count = 0;
volatile uint16_t identify_time = IDENTIFY_TIME;
uint8_t identify_channel = 0;

void commInit(void)
{
    uint8_t i;
    for (i=0;i<11;i++) read_buffer[i].i = i;
    write_buffer.current_buffer = NONE_BUFF;
    write_buffer.write_count = 0;
}

void readBit(uint8_t read_tick)
{
    uint8_t i;
    uint16_t value;
    uint32_t recipient_id;
    uint32_t header;
    uint32_t sender_id;
    uint32_t keep_alive;
    uint32_t data_frame;

    for (i=0; i<NUM_INPUTS; i++){
        // read each input that is currently receiving a message
        if ((active_input_pins[i] != 0) && (active_input_tick[i] == read_tick)){

            if (read_buffer[i].bits_left_to_read == 0){
                // The message is just starting to be read so read the message header first.
                read_buffer[i].bits_left_to_read = 5;
                read_buffer[i].callback = processMessageHeader;
            }

            // get new input value
            value = gpio_get(active_input_ports[i], active_input_pins[i]); // returns uint16 where bit position corresponds to pin number
            if (value != 0){
                value = 1;
            } else{
                value = 0;
            }

            // save new input value to buffer
            read_buffer[i].message <<= 1;
            read_buffer[i].message |= value;

            // if enough bits have been read from the message to process, then trigger callback

            // when the message buffer has read 32-bits, the message is done being read and is processed
            if (--read_buffer[i].bits_left_to_read == 0){ // done reading message
                read_buffer[i].callback(&read_buffer[i]);
                // Process message and set appropriate flags for main() or add messages to message buffer
                /*
                recipient_id = (read_buffer[i] & RECIPIENT_MASK) >> 28; // 3-bit recipient id 28
                keep_alive = (read_buffer[i] & KEEP_ALIVE_MASK) >> 22; // 6-bit keep alive 22
                sender_id = (read_buffer[i] & SENDER_MASK) >> 19; // 3-bit sender id 19
                header = (read_buffer[i] & HEADER_MASK) >> 16; // 3-bit message id 16
                data_frame = read_buffer[i] & DATA_MASK; // 16-bit data frame
                */

                /*
                if (header == PULSE){
                    dendrite_pulse_flag[i] = 1;
                } else if 

                if (header == BLINK && recipient_id == ALL){
                    if (blink_flag == 0){
                        // set blink_flag => main() will blink led
                        blink_flag = 1;
                        // forward message through network
                        addWrite(ALL_BUFF, read_buffer[i]);
                        write_buffer.source_pin = i;
                    }
                    
                } else if (recipient_id == NID){
                    addWrite(NID_BUFF, read_buffer[i]);
                } else if (header == PING){
                    if (recipient_id == DOWNSTREAM){

                        dendrite_ping_flag[i] = 1;
                        if (i % 2 != 0){
                            // excitatory
                            setAsOutput(active_input_ports[i+1], complimentary_pins[i]);
                            active_output_pins[i+1] = complimentary_pins[i];
                        } else{
                            // inhibitory
                            setAsOutput(active_input_ports[i-1], complimentary_pins[i]);
                            active_output_pins[i-1] = complimentary_pins[i];
                        }
                    } else if (recipient_id == ALL){
                        if (active_input_pins[i] == nid_pin){
                            // NID ping was received on the existing nid_pin
                            nid_ping_time = 0; // main() will reset nid pin when this reaches NID_PING_TIME
                            if (keep_alive > 0){
                                addWrite(ALL_BUFF, read_buffer[i]); // forward message to the rest of the network
                                write_buffer.source_pin = i;
                            }
                        } else if ((NID_PING_KEEP_ALIVE - keep_alive) < nid_keep_alive){
                            // the received NID ping is closer to the NID so set new NID pin
                            nid_pin = active_input_pins[i]; // nid input
                            nid_pin_out = complimentary_pins[i]; // nid output
                            nid_port = active_input_ports[i];
                            nid_port_out = active_input_ports[i];
                            nid_ping_time = 0;
                            nid_keep_alive = NID_PING_KEEP_ALIVE - keep_alive; // new keep_alive for a  NID messages
                            setAsOutput(nid_port_out, nid_pin_out);
                        }
                    }
                } else if (header == IDENTIFY){
                    if (identify_time >= IDENTIFY_TIME){
                        // set the identify_time window to 0. main() will handle the rest
                        identify_time = 0;
                        identify_channel = data_frame; // the channel to be assigned is in the data frame
                    }
                    if (keep_alive > 0){
                        // pass the message
                        addWrite(ALL_BUFF, read_buffer[i]);
                        write_buffer.source_pin = i;
                    }
                }
                */
                
                
                // deactivate input so that it doesn't keep getting read
                EXTI_PR |= active_input_pins[i];
                exti_enable_request(active_input_pins[i]);
                active_input_pins[i] = 0;
                // reset message buffer
                read_buffer[i].message = 0;
                read_buffer[i].bits_left_to_read = 5;
                read_buffer[i].callback = processMessageHeader;
            }
        }
    }
    //gpio_clear(PORT_AXON1_EX, PIN_AXON1_EX);
}

void processMessageHeader(read_buffer_t * read_buffer_ptr)
{
    uint8_t i = read_buffer_ptr->i;
    uint8_t header = (read_buffer_ptr->message & 0b01110) >> 1;

    switch (header){
        case PULSE_HEADER:
            dendrite_pulse_flag[i] = 1;
            break;
        case DOWNSTREAM_PING_HEADER:
            dendrite_ping_flag[i] = 1;
            if (i % 2 != 0){
                // excitatory
                setAsOutput(active_input_ports[i+1], complimentary_pins[i]);
                active_output_pins[i+1] = complimentary_pins[i];
            } else{
                // inhibitory
                setAsOutput(active_input_ports[i-1], complimentary_pins[i]);
                active_output_pins[i-1] = complimentary_pins[i];
            }
            break;
        case BLINK_HEADER:
            if (blink_flag == 0){
                // set blink_flag => main() will blink led
                blink_flag = 1;
                // forward message through network
                addWrite(ALL_BUFF, read_buffer_ptr->message);
                write_buffer.source_pin = i;
            }
            break;
        default:
            break;
        }
}

void addWrite(message_buffers_t buffer, uint32_t message)
{
    /*
        This function adds a new message to the write buffer.
    */
    switch (buffer){
        case DOWNSTREAM_BUFF:
            write_buffer.downstream[write_buffer.downstream_ready_count] = message;
            write_buffer.downstream_ready_count += 1;
            break;
        case NID_BUFF:
            write_buffer.nid[write_buffer.nid_ready_count] = message;
            write_buffer.nid_ready_count += 1;
            break;
        case ALL_BUFF:
            write_buffer.all[write_buffer.all_ready_count] = message;
            write_buffer.all_ready_count += 1;
            break;
        default:
            break;
    }
}

void writeBit()
{
    /*
        Pop 1-bit off the write_buffer and write it to corresponding output pins
    */
    uint8_t i;
    if (write_buffer.write_count == 33){
        // Message is done being written. Decrement the buffer that was read
        switch (write_buffer.current_buffer){
            case DOWNSTREAM_BUFF:
                for (i=0; i<2; i++){
                    write_buffer.downstream[i] = write_buffer.downstream[i+1];
                }
                write_buffer.downstream_ready_count -= 1;
                break;
            case NID_BUFF:
                for (i=0; i<4; i++){
                    write_buffer.nid[i] = write_buffer.nid[i+1];
                }
                write_buffer.nid_ready_count -= 1;
                break;
            case ALL_BUFF:
                for (i=0; i<2; i++){
                    write_buffer.all[i] = write_buffer.all[i+1];
                }
                write_buffer.all_ready_count -= 1;
                break;
            default:
                break;
        }
        write_buffer.current_buffer = NONE_BUFF;
        write_buffer.write_count = 0;
    }

    if (write_buffer.current_buffer == NONE_BUFF){
        // new message -> assign new buffer to current_buffer
        if (write_buffer.downstream_ready_count != 0){
            write_buffer.current_buffer = DOWNSTREAM_BUFF;
        } else if (write_buffer.nid_ready_count != 0){
            write_buffer.current_buffer = NID_BUFF;
        } else if (write_buffer.all_ready_count != 0){
            write_buffer.current_buffer = ALL_BUFF;
        }
    } else{
        // write 1-bit
        write_buffer.write_count += 1;
        switch (write_buffer.current_buffer){
            case DOWNSTREAM_BUFF:
                writeDownstream();
                break;
            case NID_BUFF:
                writeNID();
                break;
            case ALL_BUFF:
                writeAll();
                break;
            default:
                break;
        }
    }


}

void writeDownstream(void)
{
    uint32_t value;
    // pop next value off of buffer
    value = write_buffer.downstream[0] & 0x80000000;
    write_buffer.downstream[0] <<= 1;

    // we should have both axon out pins be on the same port that way they can be written together
    if (value != 0){
        gpio_set(PORT_AXON1_EX, PIN_AXON1_EX);
        gpio_set(PORT_AXON2_EX, PIN_AXON2_EX);
        gpio_set(PORT_AXON3_EX, PIN_AXON3_EX);
    }else{
        gpio_clear(PORT_AXON1_EX, PIN_AXON1_EX);
        gpio_clear(PORT_AXON2_EX, PIN_AXON2_EX);
        gpio_clear(PORT_AXON3_EX, PIN_AXON3_EX);
    }
}

void writeAll(void)
{
    /*
        Write to all pins on the NeuroByte except for the pin that received the message
    */

    uint8_t i;

    uint32_t value;
    // pop next bit off of buffer
    value = write_buffer.all[0] & 0x80000000;
    write_buffer.all[0] <<= 1;

    // write to all output pins except for the pin the message was received on
    for (i=0;i<11;i++){
        if (active_output_pins[i] != 0 && active_output_pins[i] != complimentary_pins[write_buffer.source_pin]){
            if (value != 0){
                gpio_set(active_output_ports[i], active_output_pins[i]);
            } else {
                gpio_clear(active_output_ports[i], active_output_pins[i]);
            }
        } 
    }
}

void writeNID(void)
{
    uint32_t value;
    value = write_buffer.nid[0] & 0x80000000;
    write_buffer.nid[0] <<= 1;

    if (value != 0){
        gpio_set(nid_port_out, nid_pin_out);
    } else{
        gpio_clear(nid_port_out, nid_pin_out);
    }
}


