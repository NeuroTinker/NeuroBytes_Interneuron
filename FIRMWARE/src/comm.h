#ifndef COMM_H_
#define COMM_H_

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/exti.h>

#include "HAL.h"

#define NUM_INPUTS 11
#define DEND_PING_TIME		200 // 1000 ms
#define	NID_PING_TIME		200 // 1000 ms
#define NID_PING_KEEP_ALIVE     32

//#define BLINK_MESSAGE           0b11111001110001110000000000000000 // (ALL) (KEEP ALIVE=7) (NID) (BLINK) (no data)
//#define PULSE_MESSAGE           0b11111000010100110000000000000000 // (DOWNSTREAM) (KEEP ALIVE=1) (UPSTREAM) (PULSE) (no data)
//#define DEND_PING               0b10010000010100010000000000000000 // (DOWNSTREAM) (KEEP ALIVE=1) (UPSTREAM) (PING) (no data)
#define NID_PING                0b10110001110000010000000000000000 // (ALL) (KEEP ALIVE=7) (NID) (PING) (no data)
#define NID_IDENTIFY_REQUEST    0b10110001110001010000000000000101 // (ALL) (KEEP ALIVE=7) (NID) (IDENTIFY) (data=0b101 : channel 2)

#define LEAD_BIT_MASK           0b10000000000000000000000000000000
#define RECIPIENT_MASK          0b01110000000000000000000000000000
#define KEEP_ALIVE_MASK         0b00001111110000000000000000000000
#define SENDER_MASK             0b00000000001110000000000000000000
#define HEADER_MASK             0b00000000000001110000000000000000
#define DATA_MASK               0b00000000000000001111111111111111

#define PULSE_HEADER                0b111 // (DOWNSTREAM) (PULSE)
#define DOWNSTREAM_PING_HEADER      0b011
#define BLINK_HEADER                0b001
#define NID_PING_HEADER             0b110
#define NID_GLOBAL_HEADER           0b100
#define DATA_HEADER                 0b010

#define PULSE_MESSAGE               0b11111000000000000000000000000000
#define DOWNSTREAM_PING_MESSAGE     0b10110000000000000000000000000000
#define BLINK_MESSAGE               0b10011000000000000000000000000000
#define NID_PING_MESSAGE            0b11101000000000000000000000000000
#define DATA_MESSAGE                0b10101000000000000000000000000000 // (NID) (KEEP ALIVE=0) (CHANNEL= NONE) (DATA) (no data)

#define IDENTIFY_COMMAND            0b000001

#define NID_PING_DATA_LENGTH        6

#define IDENTIFY_TIME       500 // 250 ms

#define LPUART1_I NUM_INPUTS + 1
/*
    This and comm.c define all communication protocol

    Messages begin with a 6-bit messsage header
    1-bit   -   lead high
    2-bit   -   recipient id
    2-bit   -   message header
    1-bit   -   header parity check
    Some messages include an (optional) data frame:
    7-bit   -   data header (optional)
    16-bit  -   data frame
    3-bit   -   data parity check

    message headers:
    0b000  -   Unused
    0b001  -   Blink (Debug)
    0b010  -   Data to NID (6-bit data header + 16-bit data packet + 3-bit parity check)
    0b011  -   Downstream Ping
    0b100  -   NID Selected Command (6-bit command header + 16-bit data-packet + 3-bit parity check)
    0b101  -   NID Global Command (6-bit command header + 1-bit parity check)
    0b110  -   NID Ping (6-bit data + 1-bit parity check)
    0b111  -   Downstream Pulse

    In general, there are three types of communication that are defined in this protocol:
    
    1. Network Interface Device (NID) broadcasting to all neurons in the network. 
    
        Messages include nid pings and identify device request. The NID commands class of messsages
        
        List of message classes:
        -{MESSAGE CLASS}
            -{MESSAGE 1}
            -{MESSAGE 2}
                -{DATA FRAME}
            -...

        -NID_PING_CLASS
            -NID_PING
                -8-bit pass count

            6-bit   -   number of devices passed
            1-bit   -   parity check

        -NID_GLOBAL_COMMAND_CLASS
            -IDENTIFY_DEVICE
            -ALL_START_LEARNING_MODE
            -ALL_STOP_LEARNING_MODE
            -ALL_PAUSE

            6-bit   -   command header
            16-bit  -   data packet
            3-bit   -   parity check

        

    2. Selected neurons sending data to the NID.

    3. Upstream neurons sending pulses to downstream neurons (axon -> dendrite).
*/

typedef struct{
    uint8_t header; // 0b1XXYY -- XX = recipient ID ; YY = message ID
    uint8_t data_header_length;
    uint8_t data_frame_length;
    uint8_t data_parity_count;
} message_class_t;


// unique pins: 0,1,3,4,5,6,7,8,9,10,13,14,15
// working dendrites: 1,2,5
// pin numbers 0*,1*,2,3*,4,5*,6*,7*,8,9,10,13,14*,15* (asterisk indicates repeated pin)
// total 14 unique pin numbers and 8 repeated pins


typedef enum{
    NID         =   0b00,
    DOWNSTREAM  =   0b11,
    ALL         =   0b01,
    SELECTED    =   0b10
} device_identifiers;

typedef enum{
    PING        =   0b00,
    PULSE       =   0b111,
    BLINK       =   0b01,
    DATA        =   0b11,
    IDENTIFY    =   0b10
} message_identifiers;

typedef enum{
    NONE_BUFF,
    DOWNSTREAM_BUFF,
    ALL_BUFF,
    NID_BUFF
} message_buffers_t;

typedef enum{
    CURRENT     =   0b0001,
    DEND1       =   0b0011,
    DEND2       =   0b0010,
    DEND3       =   0b0100,
    DEND4       =   0b0101
} parameter_identifiers;

typedef struct{
    message_buffers_t   current_buffer;
    uint8_t             write_count;
    uint32_t            downstream[3];
    uint8_t             downstream_ready_count;
    uint32_t            nid[5];
    uint8_t             nid_ready_count;
    uint32_t            all[5];
    uint8_t             all_ready_count;
    uint8_t             source_pin;
} write_buffer_t;

typedef struct read_buffer_t read_buffer_t;
typedef bool (*read_handler_t) (read_buffer_t *);
typedef struct read_buffer_t{
    uint8_t i;
    uint32_t message;
    uint8_t bits_left_to_read;
    read_handler_t callback;
} read_buffer_t;

extern uint16_t complimentary_pins[11];
extern volatile uint16_t active_input_pins[11];
extern uint32_t active_input_ports[11];
extern volatile uint16_t active_output_pins[11];
extern uint32_t active_output_ports[11];
extern volatile uint8_t active_input_tick[11];

// flags for main()
extern volatile uint8_t dendrite_pulse_flag[11];
extern volatile uint8_t blink_flag;
extern volatile uint8_t dendrite_ping_flag[11];

extern volatile uint32_t nid_ping_time;
extern volatile uint8_t nid_distance;

extern volatile uint16_t nid_pin;
extern volatile uint16_t nid_pin_out;

extern volatile uint16_t identify_time;
extern uint8_t identify_channel;

void readBit(uint8_t read_tick); // The readInputs() function reads the next bit for active inputs

void writeBit(void);

void writeAll(void);
void writeDownstream(void);

bool processMessageHeader(read_buffer_t * read_buffer_ptr);
bool processNIDPing(read_buffer_t * read_buffer_ptr);
bool processGlobalCommand(read_buffer_t * read_buffer_ptr);
bool processDataMessage(read_buffer_t * read_buffer_ptr);

// received message handlers
void receivePulse(uint32_t message);
void receiveUpstreamKeepAlive(uint32_t message, uint16_t pin);
void receiveUpstreamPulse(uint32_t message, uint16_t pin);
void receiveNIDKeepAlive(uint32_t message, uint16_t pin);
void receiveNIDBlink(void);

void addWrite(message_buffers_t buffer, uint32_t message);
void commInit(void);

void addNIDWrite(uint32_t message);
void writeNIDByte(uint8_t byte);
uint16_t readNIDByte(void);
void readNID(void);
void writeNID(void);
#endif
