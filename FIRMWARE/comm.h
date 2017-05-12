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


#define BLINK_MESSAGE           0b10110001110001110000000000000000 // (ALL) (KEEP ALIVE=7) (NID) (BLINK) (no data)
#define PULSE_MESSAGE           0b10010000010100110000000000000000 // (DOWNSTREAM) (KEEP ALIVE=1) (UPSTREAM) (PULSE) (no data)
#define DATA_MESSAGE            0b10000000000001000000000000000000 // (NID) (KEEP ALIVE=0) (CHANNEL= NONE) (DATA) (no data)
#define DEND_PING               0b10010000010100010000000000000000 // (DOWNSTREAM) (KEEP ALIVE=1) (UPSTREAM) (PING) (no data)
#define NID_PING                0b10110001110000010000000000000000 // (ALL) (KEEP ALIVE=7) (NID) (PING) (no data)
#define NID_IDENTIFY_REQUEST    0b10110001110001010000000000000101 // (ALL) (KEEP ALIVE=7) (NID) (IDENTIFY) (data=0b101 : channel 2)

#define LEAD_BIT_MASK           0b10000000000000000000000000000000
#define RECIPIENT_MASK          0b01110000000000000000000000000000
#define KEEP_ALIVE_MASK         0b00001111110000000000000000000000
#define SENDER_MASK             0b00000000001110000000000000000000
#define HEADER_MASK             0b00000000000001110000000000000000
#define DATA_MASK               0b00000000000000001111111111111111

#define IDENTIFY_TIME       200 // 1000 ms

/*
    This and comm.c define all communication protocol

    Messages are 32-bit packets:
    1-bit   -   lead high
    3-bit   -   recipient id
    6-bit   -   keep-alive 
    3-bit   -   sender id
    3-bit   -   header
    16-bit  -   data frame

    In general, there are three types of communication that are defined in this protocol:
    
    1. Network Interface Device (NID) broadcasting to all neurons in the network. 

    2. Selected neurons sending data to the NID.

    3. Upstream neurons sending pulses to downstream neurons (axon -> dendrite).
*/

/*
typedef enum{
    PORT_AXON_IN    =   GPIOA,
    PORT_AXON_OUT   =   GPIOA,
    PORT_DEND1_EX   =   GPIOB,
    PORT_DEND1_IN   =   GPIOB,
    PORT_DEND2_EX   =   GPIOA,
    PORT_DEND2_IN   =   GPIOA,
    PORT_DEND3_EX   =   GPIOA,
    PORT_DEND3_IN   =   GPIOC,
    PORT_DEND4_EX   =   GPIOB,
    PORT_DEND4_IN   =   GPIOB,
    PORT_DEND5_EX   =   GPIOB,
    PORT_DEND5_IN   =   GPIOB
} gpio_port;
*/
// unique pins: 0,1,3,4,5,6,7,8,9,10,13,14,15
// working dendrites: 1,2,5
// pin numbers 0*,1*,2,3*,4,5*,6*,7*,8,9,10,13,14*,15* (asterisk indicates repeated pin)
// total 14 unique pin numbers and 8 repeated pins
/*
typedef enum{
    PIN_AXON_IN     =   GPIO9,
    PIN_AXON_OUT    =   GPIO10,
    PIN_DEND1_EX    =   GPIO1,
    PIN_DEND1_IN    =   GPIO0, 
    PIN_DEND2_EX    =   GPIO7,
    PIN_DEND2_IN    =   GPIO6,
    PIN_DEND3_EX    =   GPIO1,
    PIN_DEND3_IN    =   GPIO14,
    PIN_DEND4_EX    =   GPIO7,
    PIN_DEND4_IN    =   GPIO6,
    PIN_DEND5_EX    =   GPIO4,
    PIN_DEND5_IN    =   GPIO3
} gpio_pin;
*/
typedef enum{
    NID         =   0b000,
    DOWNSTREAM  =   0b001,
    UPSTREAM    =   0b010,
    ALL         =   0b011,
    SELECTED1   =   0b100,
    SELECTED2   =   0b101,
    SELECTED3   =   0b110,
    SELECTED4   =   0b111
} device_identifiers;

typedef enum{
    PING        =   0b001,
    PULSE       =   0b011,
    BLINK       =   0b111,
    DATA        =   0b100,
    IDENTIFY    =   0b101
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
    message_buffers_t     current_buffer;
    uint8_t             write_count;
    uint32_t            downstream[3];
    uint8_t             downstream_ready_count;
    uint32_t            nid[5];
    uint8_t             nid_ready_count;
    uint32_t            all[5];
    uint8_t             all_ready_count;
    uint8_t             source_pin;
} write_buffer_t;


extern uint16_t complimentary_pins[11];

extern volatile uint16_t active_input_pins[11];

extern uint32_t active_input_ports[11];

extern volatile uint16_t active_output_pins[11];

extern uint32_t active_output_ports[11];

// write outputs from message buffer

extern volatile uint32_t downstream_write_buffer; // buffer just for messages traveling downstream through axon
extern volatile uint8_t downstream_write_buffer_ready;

extern volatile uint32_t nid_write_buffer; // buffer just for messages being sent to NID
extern volatile uint8_t nid_write_buffer_ready;

extern volatile uint32_t all_write_buffer;
extern volatile uint8_t all_write_buffer_ready;

extern uint8_t write_count; // Incremented after each bit is written. Write new message after 32-bits.

extern volatile uint8_t dendrite_pulse_flag[11];
extern volatile uint8_t blink_flag;
extern volatile uint8_t dendrite_ping_flag[11];

extern volatile uint32_t nid_ping_time;
extern volatile uint32_t nid_keep_alive;

extern volatile uint16_t nid_pin;
extern volatile uint16_t nid_pin_out;

extern volatile uint16_t identify_time;
extern uint8_t identify_channel;

void readInputs(void); // The readInputs() function reads incoming messages and decides if it the data frame should be sent to a handler based off of sender id and message id.

void write(void);
void writeAll(void);
void writeDownstream(void);
void writeNID(void);

// received message handlers
void receivePulse(uint32_t message);
void receiveUpstreamKeepAlive(uint32_t message, uint16_t pin);
void receiveUpstreamPulse(uint32_t message, uint16_t pin);
void receiveNIDKeepAlive(uint32_t message, uint16_t pin);
void receiveNIDBlink(void);
void addWrite(message_buffers_t buffer, uint32_t message);
void commInit(void);

// send message handlers
//void sendDownstream(uint16_t data);
//void passMessage(uint32_t message);


#endif