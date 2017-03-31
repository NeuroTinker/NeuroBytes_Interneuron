
#include "comm.h"

void readInputs(void)
{
    uint8_t i;
    uint16_t value;
    uint8_t sender_id;
    uint8_t message_id;

    // It might be faster to read whole ports at a time using gpio_port_read() instead of individually reading pins.

    for (i=0; i<NUM_INPUTS; i++){

        if (active_input_pins[i] != 0){

            // get new input value
            value = gpio_get(active_input_ports[i], active_input_pins[i]); // returns uint16 where bit position corresponds to pin number
            value >>= active_input_pins[i];

            // save new input value to buffer
            message_buffer[i] <<= 1;
            message_buffer[i] |= value;

            if (++message_buffer_count[i] == 32){ // done reading message

                // Process message and send to appropriate handler
                // It might be better to buffer the message and process it later

                sender_id = message_buffer[i] >> 28; // 4-bit sender id
                message_id = (message_buffer[i] >> 16) | 0b111111; // 6-bit message id

                switch (sender_id){
                    case N1:
                        //passMessage(message_buffer[i]);
                        break;
                    case N2:
                        //passMessage(message_buffer[i]);
                        break;
                    case N3:
                        //passMessage(message_buffer[i]);
                        break;
                    case N4:
                        //passMessage(message_buffer[i]);
                        break;
                    case UPSTREAM:
                        if (message_id == KEEP_ALIVE){
                            //receiveUpstreamKeepAlive(message_buffer[i], active_input_pins[i]);
                        } else if (message_id == PULSE){
                            //receiveUpstreamPulse(message_buffer[i], active_input_pins[i]);
                        }
                        break;
                    case NID:
                        switch (message_id){
                            case KEEP_ALIVE:
                                //receiveNIDKeepAlive(message_buffer[i], active_input_pins[i]);
                                break;
                            case BLINK:
                                //receiveNIDBlink();
                                break;
                            default:
                                break;
                        }
                        break;
                    default:
                        break;
                }

                // reset input interrupt
                exti_reset_request(active_input_ports[i]);

                // deactivate input so that it doesn't keep getting read
                active_input_pins[i] = 0;

                // reset message buffer
                message_buffer[i] = 0;
                message_buffer_count[i] = 0;
            }
        }
    }
}

void writeDownstream(void)
{
    uint8_t value;
    value = downstream_write_buffer | 0b1;

    if (value == 1){
        gpio_set(PORT_AXON_OUT, PIN_AXON_OUT);
    }else{
        gpio_clear(PORT_AXON_OUT, PIN_AXON_OUT);
    }

    // this goes outside of the function
    if (++write_count == 32){
        write_count = 0;
    }
}

void initializeVars(void)
{
    uint8_t i;

    for (i=0; i<11; i++){
        message_buffer_count[i] = 0;
    }

    downstream_write_buffer_ready = 0;
    write_buffer_ready = 0;
    write_count = 0;
    nid_write_buffer_ready = 0;
    messages_count = 0;

}