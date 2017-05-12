
#include "comm.h"
#include "HAL.h"

write_buffer_t write_buffer;

uint32_t message_buffer[11] = {0,0,0,0,0,0,0,0,0,0,0};
uint8_t message_buffer_count[11];

extern volatile uint16_t active_input_pins[11] = {0,0,0,0,0,0,0,0,0,0,0};

extern volatile uint16_t active_output_pins[11] = {PIN_AXON_OUT,0,0,0,0,0,0,0,0,0,0};

extern volatile uint32_t dendrite_pulses[5] = {0,0,0,0,0};
extern volatile uint8_t dendrite_pulse_count = 0;

extern volatile uint8_t blink_flag = 0;

extern volatile uint32_t nid_ping_time = 0;

extern volatile uint16_t nid_pin = 0;
uint32_t nid_port = 0;
extern volatile uint16_t nid_pin_out = 0;
uint32_t nid_port_out = 0;
uint8_t nid_i      =    4;
extern volatile uint32_t  nid_keep_alive = NID_PING_KEEP_ALIVE;


/* 
All available input pins are
    = {
        PIN_AXON_IN,
        PIN_DEND1_EX,
        PIN_DEND1_IN, 
        PIN_DEND2_EX,
        PIN_DEND2_IN,
        PIN_DEND3_EX,
        PIN_DEND3_IN,
        PIN_DEND4_EX,
        PIN_DEND4_IN,
        PIN_DEND5_EX,
        PIN_DEND5_IN
    };
*/

extern uint32_t active_input_ports[11] = {
    PORT_AXON_IN,
    PORT_DEND1_EX,
    PORT_DEND1_IN,
    PORT_DEND2_EX,
    PORT_DEND2_IN,
    PORT_DEND3_EX,
    PORT_DEND3_IN,
    PORT_DEND4_EX,
    PORT_DEND4_IN,
    PORT_DEND5_EX,
    PORT_DEND5_IN
};

extern uint32_t active_output_ports[11] = {
    PORT_AXON_OUT,
    PORT_DEND1_EX,
    PORT_DEND1_IN,
    PORT_DEND2_EX,
    PORT_DEND2_IN,
    PORT_DEND3_EX,
    PORT_DEND3_IN,
    PORT_DEND4_EX,
    PORT_DEND4_IN,
    PORT_DEND5_EX,
    PORT_DEND5_IN
};

extern uint16_t complimentary_pins[11] = {
    0,
    PIN_DEND1_IN,
    PIN_DEND1_EX,
    PIN_DEND2_IN,
    PIN_DEND2_EX,
    PIN_DEND3_IN,
    PIN_DEND3_EX,
    PIN_DEND4_IN,
    PIN_DEND4_EX,
    PIN_DEND5_IN,
    PIN_DEND5_EX
};

// complimentary port is same port

extern volatile uint32_t downstream_write_buffer = 0;
extern volatile uint8_t downstream_write_buffer_ready = 0;
extern volatile uint32_t all_write_buffer = 0;
extern volatile uint8_t all_write_buffer_ready = 0;
extern volatile uint32_t nid_write_buffer = 0;
extern volatile uint8_t nid_write_buffer_ready = 0;
extern volatile uint8_t dendrite_pulse_flag[11] = {0,0,0,0,0,0,0,0,0,0,0};
extern volatile uint8_t dendrite_ping_flag[11] = {0,0,0,0,0,0,0,0,0,0,0};
uint8_t write_count = 0;
extern volatile uint16_t identify_time = 0;
extern uint8_t identify_channel = 0;

void commInit(void)
{
    write_buffer.current_buffer = NONE_BUFF;
    write_buffer.write_count = 0;
}

void readInputs(void)
{
    uint8_t i;
    uint16_t value;
    uint32_t recipient_id;
    uint32_t header;
    uint32_t sender_id;
    uint32_t keep_alive;
    uint32_t data_frame;

    // It might be faster to read whole ports at a time using gpio_port_read() instead of individually reading pins.
    for (i=0; i<NUM_INPUTS; i++){

        if (active_input_pins[i] != 0){
            // get new input value
            value = gpio_get(active_input_ports[i], active_input_pins[i]); // returns uint16 where bit position corresponds to pin number
            if (value != 0){
                value = 1;
            } else{
                value = 0;
            }

            // save new input value to buffer
            message_buffer[i] <<= 1;
            message_buffer[i] |= value;

            if (++message_buffer_count[i] == 32){ // done reading message
                // Process message and send to appropriate handler
                // It might be better to buffer the message and process it later
                //gpio_toggle(PORT_AXON_OUT,PIN_AXON_OUT); // debug
                recipient_id = (message_buffer[i] & RECIPIENT_MASK) >> 28; // 3-bit recipient id 28
                keep_alive = (message_buffer[i] & KEEP_ALIVE_MASK) >> 22; // 6-bit keep alive 22
                sender_id = (message_buffer[i] & SENDER_MASK) >> 19; // 3-bit sender id 19
                header = (message_buffer[i] & HEADER_MASK) >> 16; // 3-bit message id 16
                data_frame = message_buffer[i] & DATA_MASK; // 16-bit data frame

                // decrement keep alive
                //message_buffer[i] = (((keep_alive - 1) << 22) & message_buffer[i]) | (message_buffer[i] & ~KEEP_ALIVE_MASK);
                message_buffer[i] = (((keep_alive - 1) << 22) & KEEP_ALIVE_MASK) | (message_buffer[i] & ~KEEP_ALIVE_MASK);
                // debug send downstream
                
                /*
                downstream_write_buffer = message_buffer[i];
                downstream_write_buffer_ready = 1;
                */

                //addWrite(NID_BUFF, message_buffer[i]);
                
                if (header == BLINK && recipient_id == ALL){
                    blink_flag = 1;
                    
                    addWrite(ALL_BUFF, message_buffer[i]);
                    write_buffer.source_pin = i;
                    
                } else if (recipient_id == NID){
                    addWrite(NID_BUFF, message_buffer[i]);
                  
                    //nid_write_buffer = message_buffer[i] | ((keep_alive - 1) << 22);
                    //nid_write_buffer_ready = 1;
                } else if (header == PING){
                    if (recipient_id == DOWNSTREAM){
                        // receive upstream keep-alive
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
                        // receive NID keep-alive
                        /*
                        nid_pin = active_input_pins[i];
                            nid_pin_out = complimentary_pins[i];
                            nid_port = active_input_ports[i];
                            nid_port_out = active_input_ports[i];
                            nid_ping_time += NID_PING_TIME;
                            nid_keep_alive = keep_alive;
                            setAsOutput(nid_port_out, nid_pin_out);
                            */
                        if (active_input_pins[i] == nid_pin){
                            nid_ping_time = NID_PING_TIME;
                            if (keep_alive > 0){
                                addWrite(ALL_BUFF, message_buffer[i]);
                                write_buffer.source_pin = i;
                            }
                            //all_write_buffer = message_buffer[i] | ((keep_alive -1) << 22); // decrement keep-alive and pass
                            //all_write_buffer_ready = 1;
                        } else if ((NID_PING_KEEP_ALIVE - keep_alive) < nid_keep_alive){
                            nid_pin = active_input_pins[i];
                            nid_pin_out = complimentary_pins[i];
                            nid_port = active_input_ports[i];
                            nid_port_out = active_input_ports[i];
                            nid_ping_time = NID_PING_TIME;
                            nid_keep_alive = NID_PING_KEEP_ALIVE - keep_alive;
                            setAsOutput(nid_port_out, nid_pin_out);
                        }
                    }
                } else if (header == PULSE){
                    dendrite_pulse_flag[i] = 1;
                } else if (header == IDENTIFY){
                    if (identify_time == 0){
                        identify_time = IDENTIFY_TIME;
                        identify_channel = data_frame;
                    }
                    if (keep_alive > 0){
                        addWrite(ALL_BUFF, message_buffer[i]);
                        write_buffer.source_pin = i;
                    }
                }
                
                
                
                // deactivate input so that it doesn't keep getting read
                EXTI_PR |= active_input_pins[i];
                active_input_pins[i] = 0;
                // reset message buffer
                message_buffer[i] = 0;
                message_buffer_count[i] = 0;
            }
        }
    }
}

void addWrite(message_buffers_t buffer, uint32_t message)
{
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

void write()
{
    uint8_t i;
    if (write_buffer.write_count == 33){
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
        if (write_buffer.downstream_ready_count != 0){
            write_buffer.current_buffer = DOWNSTREAM_BUFF;
        } else if (write_buffer.nid_ready_count != 0){
            write_buffer.current_buffer = NID_BUFF;
        } else if (write_buffer.all_ready_count != 0){
            write_buffer.current_buffer = ALL_BUFF;
        }
    } else{
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
    //value = downstream_write_buffer | 0b1;
    //value = (downstream_write_buffer >> 31) | 0b1;
    value = write_buffer.downstream[0] & 0x80000000;
    write_buffer.downstream[0] <<= 1;

    // we should have both axon out pins be on the same port that way they can be written together
    if (value != 0){
        gpio_set(PORT_AXON_OUT, PIN_AXON_OUT);
    }else{
        gpio_clear(PORT_AXON_OUT, PIN_AXON_OUT);
    }
}

void writeAll(void)
{
    uint8_t i;

    uint32_t value;
    value = write_buffer.all[0] & 0x80000000;
    write_buffer.all[0] <<= 1;
    active_output_ports[0] = PORT_AXON_OUT;
    active_output_pins[0] = PIN_AXON_OUT;
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


