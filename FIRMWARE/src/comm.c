
#include "comm.h"
#include "HAL.h"

write_buffer_t write_buffer;
read_buffer_t read_buffer[11] = {
    [0 ... 10] = { .message=0, .bits_left_to_read=5, .callback=processMessageHeader}
}; // fast 'gcc' way to initialize the whole array of read_buffer_t struct

//uint32_t read_buffer[11] = {0,0,0,0,0,0,0,0,0,0,0};
//uint8_t read_buffer_bits_left[11] = 0,0,0,0,0,0,0,0,0,0,0};
//void (*read_buffer_callback[11]) (uint32_t);

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

uint32_t lpuart_message;
uint8_t lpuart_count;


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

uint32_t complimentary_ports[11] = {
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
    for (i=0;i<11;i++) read_buffer[i].i = i;
    write_buffer.current_buffer = NONE_BUFF;
    write_buffer.write_count = 0;
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
                    read_buffer[i].bits_left_to_read = 5;
                    read_buffer[i].callback = processMessageHeader;
                }
            }
        }
    }
}

bool processMessageHeader(read_buffer_t * read_buffer_ptr)
{
    uint8_t i = read_buffer_ptr->i;
    uint8_t header = (read_buffer_ptr->message & 0b01110) >> 1;

    switch (header){
        case PULSE_HEADER:
            dendrite_pulse_flag[i] = 1;
            break;
        case DOWNSTREAM_PING_HEADER:
            /* Check to make sure it's a dendrite */
            if (i > 2){
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
                active_output_pins[i] = 0; // might not be neccesary
            }
            break;
        case BLINK_HEADER:
            if ((blink_flag == 0) && (i == nid_i)){
                // set blink_flag => main() will blink led
                blink_flag = 1;
                // forward message through network
                addWrite(ALL_BUFF, BLINK_MESSAGE);
                write_buffer.source_pin = i;
            }
            break;
        case NID_PING_HEADER:
            // NID ping received. Read the distance packet and then process it.
            read_buffer_ptr->bits_left_to_read = 7;
            read_buffer_ptr->callback = processNIDPing;
            return true;
            break;
        case NID_GLOBAL_HEADER:
            if (i == nid_i){
                read_buffer_ptr->bits_left_to_read = 7;
                read_buffer_ptr->callback = processGlobalCommand;
            }
            return true;
            break;
        case DATA_HEADER:
            read_buffer_ptr->bits_left_to_read = 27;
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
    addWrite(NID_BUFF, read_buffer_ptr->message);
    blink_flag = 1;
    return false;
}

bool processGlobalCommand(read_buffer_t * read_buffer_ptr)
{
    uint8_t i = read_buffer_ptr->i;
    uint32_t message;

    uint8_t command = (read_buffer_ptr->message & 0b1111110) >> 1;

    switch (command){
        case IDENTIFY_COMMAND:
            identify_time = 0;
            identify_channel = 1;
            break;
        default:
            break;
    }
    message = read_buffer_ptr->message << 20;
    addWrite(ALL_BUFF, message);
    write_buffer.source_pin = i;
    return false;
}

bool processNIDPing(read_buffer_t * read_buffer_ptr)
{
    uint32_t message;
    uint8_t i = read_buffer_ptr->i;
    static uint8_t ping_count = 0; // ping_count locks the nid to a certain pin after receiving enough least-distance pings

    uint8_t distance = (read_buffer_ptr->message & 0b1111110) >> 1;

    if (i != nid_i){
        // NID ping was  not received on the existing nid_pin
        if (distance < nid_distance){
            if (ping_count < 3){
                // the received NID ping is closer to the NID so set new NID pin
                nid_i = i; 
                if (nid_i != LPUART1_I){
                    nid_pin = active_input_pins[i]; // nid input
                    nid_pin_out = complimentary_pins[i]; // nid output
                    nid_port = active_input_ports[i];
                    nid_port_out = complimentary_ports[i];
                    setAsOutput(nid_port_out, nid_pin_out);
                }else{
                    nid_port_out = LPUART1;
                }
                nid_distance = distance;
                ping_count = 0;
            } else {
                ping_count -= 1;
            }
        } else {
            return false;
        }
    }

    if (distance == nid_distance){
        if (ping_count < 5) ping_count += 1;
        nid_ping_time = 0; // main() will reset nid pin when this reaches NID_PING_TIME
        message = NID_PING_MESSAGE | ((distance+1)<<(32-5-NID_PING_DATA_LENGTH)); // DEBUG need typecasting?
        write_buffer.source_pin = i;    
        addWrite(ALL_BUFF, message); // forward message to the rest of the network
    } else if (--ping_count == 0){
        nid_distance = 100;
        nid_pin_out = 0;
        nid_i = 13;
    }

    return false;
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
            if (nid_i == LPUART1_I){
                lpuart_message = message;
                lpuart_count = 0;
                /* usart_enable_tx_interrupt(LPUART1); */
                /* USART_CR1(LPUART1) &= ~(USART_CR1_TE); */
                writeNID(); // write first byte to NID and TXE interrupt will write the rest of the message
            }else{
                write_buffer.nid[write_buffer.nid_ready_count] = message;
                write_buffer.nid_ready_count += 1;
            }
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
    if (lpuart_count < 4 && nid_i == LPUART1_I){
        writeNID();//debug
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
        if (lpuart_count++ <= 4){
            value = lpuart_message & (0xFF << 0x18);
            value >>= 0x18;
            lpuart_message <<= 0x8;
            writeNIDByte(value);
        }
        /* else{ */
            /* USART_CR1(LPUART1) &= ~(USART_CR1_TE); */
            /* usart_disable_tx_interrupt(LPUART1); */
        /* } */
    } else{
        value = write_buffer.nid[0] & 0x80000000;
        write_buffer.nid[0] <<= 1;

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
    read_buffer_t nid_read_buffer = { .message=0, .bits_left_to_read=5, .callback=processMessageHeader, .i=LPUART1_I};
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

        nid_read_buffer.message = 0;
        nid_read_buffer.bits_left_to_read = 5;
        nid_read_buffer.callback = processMessageHeader;
        i = 0;
    }
}

