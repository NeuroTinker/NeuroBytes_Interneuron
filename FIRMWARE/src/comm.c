
#include "comm.h"
#include "HAL.h"

write_buffer_t write_buffer;
read_buffer_t read_buffer[11] = {
    [0 ... 10] = { .message=0, .bits_left_to_read=4, .callback=processMessageHeader}
}; // fast 'gcc' way to initialize the whole array of read_buffer_t struct


volatile uint16_t active_input_pins[11] = {[0 ... 10] = 0};

volatile uint8_t active_input_tick[11] = {[0 ... 10] = 0};

volatile uint16_t active_output_pins[11] = {PIN_AXON1_EX, PIN_AXON2_EX, PIN_AXON3_EX, [3 ... 10] = 0};

volatile uint32_t dendrite_pulses[4] = {0,0,0,0};
volatile uint8_t dendrite_pulse_count = 0;

volatile uint8_t blink_flag = 0;

volatile uint32_t nid_ping_time = 0;

volatile uint16_t nid_pin = 0;
uint32_t nid_port = 0;
volatile uint16_t nid_pin_out = 0;
uint32_t nid_port_out = 0;
uint8_t nid_i      =    13;
volatile uint8_t  nid_distance = 100; // max uint8_t
volatile uint8_t closer_ping_count = 0;
volatile uint8_t closer_distance = 100;

const message_t pulse_message = {.length=4, .message=PULSE_HEADER};
const message_t downstream_ping_message = {.length=4, .message=DOWNSTREAM_PING_HEADER};
const message_t blink_message = {.length=4, .message=BLINK_HEADER};


/* 
All available input pins are:

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

const uint16_t complimentary_pins[11] = {
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

const uint32_t complimentary_ports[11] = {
    PORT_AXON1_EX,
    PORT_AXON2_EX,
    PORT_AXON3_EX,
    PORT_DEND1_IN,
    PORT_DEND1_EX,
    PORT_DEND2_IN,
    PORT_DEND2_EX,
    PORT_DEND3_IN,
    PORT_DEND3_EX,
    PORT_DEND4_IN,
    PORT_DEND4_EX
};

volatile uint8_t dendrite_pulse_flag[11] = {[0 ... 10] = 0};
volatile uint8_t dendrite_ping_flag[11] = {[0 ... 10] = 0};
uint8_t write_count = 0;
volatile uint16_t identify_time = IDENTIFY_TIME;
uint8_t identify_channel = 0;

void commInit(void)
{
    uint8_t i;
    for (i=0;i<NUM_INPUTS;i++) read_buffer[i].i = i;
    write_buffer.current_buffer = NONE_BUFF;
    write_buffer.write_count = 0;
    write_buffer.num_bits_to_write = 1;
}

void readBit(uint8_t read_tick)
{
    uint8_t i;
    uint16_t value;

    for (i=0; i<NUM_INPUTS; i++){
        // read each input that is currently receiving a message
        if ((active_input_pins[i] != 0) && (active_input_tick[i] == read_tick)){

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

            if (--read_buffer[i].bits_left_to_read == 0){ // done reading message
                // execute message callback. returns true if there is more to be read
                if (!read_buffer[i].callback(&read_buffer[i])){
                     // deactivate input so that it doesn't keep getting read
                    EXTI_PR |= active_input_pins[i];
                    exti_enable_request(active_input_pins[i]);
                    active_input_pins[i] = 0;
                    // reset message buffer
                    read_buffer[i].message = 0;
                    read_buffer[i].bits_left_to_read = 4;
                    read_buffer[i].callback = processMessageHeader;
                }
            }
        }
    }
}

bool processMessageHeader(read_buffer_t * read_buffer_ptr)
{
    uint8_t i = read_buffer_ptr->i;
    uint8_t header = read_buffer_ptr->message & 0b1111;

    switch (header){
        case PULSE_HEADER:
            dendrite_pulse_flag[i] = 1;
            break;
        case DOWNSTREAM_PING_HEADER:
            /* Check to make sure it's a dendrite */
            if (i >= NUM_AXONS){
                dendrite_ping_flag[i] = 1;
                setAsOutput(complimentary_ports[i], complimentary_pins[i]);
                active_output_pins[COMPLIMENTARY_I(i)] = complimentary_pins[i];
                active_output_pins[i] = 0; // might not be neccesary
            }
            break;
        case BLINK_HEADER:
            if ((blink_flag == 0) && (i == nid_i)){
                // set blink_flag => main() will blink led
                blink_flag = 1;
                // forward message through network
                addWrite(ALL_BUFF, blink_message);
                write_buffer.source_pin = i;
            }
            break;
        case NID_PING_HEADER:
            // NID ping received. Read the distance packet and then process it.
            read_buffer_ptr->bits_left_to_read = 6;
            read_buffer_ptr->callback = processNIDPing;
            return true;
            break;
        case NID_GLOBAL_HEADER:
            if (i == nid_i){
                read_buffer_ptr->bits_left_to_read = 6;
                read_buffer_ptr->callback = processGlobalCommand;
                return true;
            }
            break;
        case DATA_HEADER:
            read_buffer_ptr->bits_left_to_read = 28;
            read_buffer_ptr->callback = processDataMessage;
            return true;
            break;
        default:
            break;
    }
    return false;
}

bool processDataMessage(read_buffer_t * read_buffer_ptr)
{
    const message_t frwd_message = {.length=32, .message=read_buffer_ptr->message};
    addWrite(NID_BUFF, frwd_message);
    return false;
}

bool processGlobalCommand(read_buffer_t * read_buffer_ptr)
{
    uint8_t command = read_buffer_ptr->message & 0b111111;

    switch (command){
        case IDENTIFY_COMMAND:
            read_buffer_ptr->bits_left_to_read = 3;
            read_buffer_ptr->callback = processIdentifyCommand;
            return true;
            break;
        default:
            break;
    }
    const message_t frwd_message = {.length=10, .message=read_buffer_ptr->message};
    addWrite(ALL_BUFF, frwd_message);
    return false;
}

bool processIdentifyCommand(read_buffer_t * read_buffer_ptr)
{
    identify_channel = read_buffer_ptr->message & 0b111;
    identify_time = 0;
    const message_t frwd_message = {.length=13, .message=read_buffer_ptr->message};
    addWrite(ALL_BUFF, frwd_message);
    return false;
}

bool processNIDPing(read_buffer_t * read_buffer_ptr)
{
    uint8_t i = read_buffer_ptr->i;

    uint8_t distance = read_buffer_ptr->message & 0b111111;

    if (i != nid_i){
        // NID ping was not received on the existing nid_pin
        if (distance < nid_distance){
            if (distance < closer_distance){
                closer_ping_count = 1;
                closer_distance = distance;
            } else if (distance == closer_distance){
                if (++closer_ping_count >= CLOSER_PING_COUNT){
                    // the received NID ping is closer to the NID so set new NID pin
                    nid_i = i; 
                    if (nid_i != LPUART1_I){
                        nid_pin = active_input_pins[i]; // nid input
                        nid_pin_out = complimentary_pins[i]; // nid output
                        nid_port = active_input_ports[i];
                        nid_port_out = complimentary_ports[i];
                        //setAsOutput(nid_port_out, nid_pin_out);
                    }else{
                        nid_port_out = LPUART1;
                    }
                    nid_distance = distance;
                }
            }          
        } else {
            return false;
        }
    }

    const message_t frwd_message = {
        .length=10,
        .message= ((read_buffer_ptr->message & 0b1111000000) | (distance+1)) // increment nid distance before forwarding it
    };

    if (distance == nid_distance){
        nid_ping_time = NID_PING_TIME; // main() will reset nid pin when this reaches NID_PING_TIME
        write_buffer.source_pin = i;    
        addWrite(ALL_BUFF, frwd_message); // forward message to the rest of the network
    }

    return false;
}

void addWrite(message_buffers_t buffer, const message_t message)
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

void writeBit(void)
{
    /*
        Pop 1-bit off the write_buffer and write it to corresponding output pins
    */
    uint8_t i;
    if (write_buffer.write_count >= write_buffer.num_bits_to_write){
        // Message is done being written. Decrement the buffer that was read
        switch (write_buffer.current_buffer){
            case DOWNSTREAM_BUFF:
                for (i=0; i<(DOWNSTREAM_BUFFSIZE-1); i++){
                    write_buffer.downstream[i] = write_buffer.downstream[i+1];
                }
                write_buffer.downstream_ready_count -= 1;
                break;
            case NID_BUFF:
                for (i=0; i<(NID_BUFFSIZE-1); i++){
                    write_buffer.nid[i] = write_buffer.nid[i+1];
                }
                write_buffer.nid_ready_count -= 1;
                break;
            case ALL_BUFF:
                for (i=0; i<(ALL_BUFFSIZE-1); i++){
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
        // new message. so assign new buffer to current_buffer
        if (write_buffer.downstream_ready_count != 0){
            write_buffer.current_buffer = DOWNSTREAM_BUFF;
            write_buffer.num_bits_to_write = write_buffer.downstream[0].length + 1;
        } else if (write_buffer.nid_ready_count != 0){ 
            write_buffer.current_buffer = NID_BUFF;
            write_buffer.num_bits_to_write = write_buffer.nid[0].length + 1;
        } else if (write_buffer.all_ready_count != 0){
            write_buffer.current_buffer = ALL_BUFF;
            write_buffer.num_bits_to_write = write_buffer.all[0].length + 1;
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
    uint8_t i;
    uint32_t value;
    // pop next value off of buffer
    value = NEXT_BIT(write_buffer.downstream[0]);
    write_buffer.downstream[0].message <<= 1;

    if (value != 0){
        for (i=0; i<NUM_AXONS; i++){
            gpio_set(active_output_ports[i], active_output_pins[i]);
        }
    }else{
        for (i=0; i<NUM_AXONS; i++){
            gpio_clear(active_output_ports[i], active_output_pins[i]);
        }
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
    value = NEXT_BIT(write_buffer.all[0]);
    write_buffer.all[0].message <<= 1;

    // write to all output pins except for the pin the message was received on
    for (i=0;i<NUM_INPUTS;i++){
        if ((active_output_pins[i] != 0) && (active_output_pins[i] != complimentary_pins[write_buffer.source_pin])){
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
    if (nid_port_out == LPUART1){
        value = write_buffer.nid[0].message & (0xFF << 0x18);
        value >>= 0x18;
        write_buffer.nid[0].message <<= 0x8;
        write_buffer.write_count += 8; // lpuart writes 8 bits at a time instead of 1 bit
        writeNIDByte(value);
    } else{
        value = NEXT_BIT(write_buffer.nid[0]);
        write_buffer.nid[0].message <<= 1;

        if (value != 0){
            gpio_set(nid_port_out, nid_pin_out);
        } else{
            gpio_clear(nid_port_out, nid_pin_out);
        }
    }
}

void writeNIDByte(uint8_t byte)
{
    usart_send(LPUART1, byte);
}

uint16_t readNIDByte(void)
{
    return usart_recv(LPUART1);
}

void readNID(void)
{
    static uint8_t i = 0;
    static uint32_t message = 0;
    read_buffer_t nid_read_buffer = { .message=0, .bits_left_to_read=4, .callback=processMessageHeader, .i=LPUART1_I};
    message <<= 8;
    message |= readNIDByte();
    if (++i == 4){ // TODO need to flush if nid gets disconnected
        /* Process message */
        do{
            while (nid_read_buffer.bits_left_to_read > 0){
                nid_read_buffer.bits_left_to_read -= 1;
                nid_read_buffer.message <<= 1;
                nid_read_buffer.message |= (message & 0x80000000) >> 31;
                message <<= 1;
            }
        } while(nid_read_buffer.callback(&nid_read_buffer));
        i = 0;
    }
}

