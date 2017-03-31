#ifndef COMM_H_
#define COMM_H_

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/cm3/nvic.h>
#include <libopencm3/cm3/systick.h>
#include <libopencm3/stm32/exti.h>

#define NUM_INPUTS 11

/*
    This and comm.c define all communication protocol

    Messages are 32-bit packets:
    1-bit   -   lead high
    3-bit   -   sender id
    6-bit   -   keep-alive 
    6-bit   -   message id
    16-bit  -   data frame

    In general, there are three types of communication that are defined in this protocol:
    
    1. Network Interface Device (NID) broadcasting to all neurons in the network. 

    2. Selected neurons sending data to the NID.

    3. Upstream neurons sending pulses to downstream neurons (axon -> dendrite).
*/

typedef enum {
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

// unique pins: 0,1,3,4,6,7,9,10,14
// working dendrites: 1,2,5
// pin numbers 0*,1*,2,3*,4,5*,6*,7*,8,9,10,13,14*,15* (asterisk indicates repeated pin)
// total 14 unique pin numbers and 8 repeated pins
typedef enum {
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

typedef enum{
    NID         =   0b1000, // Network Interface Device (NID) broadcasting to whole network
    N1          =   0b1001, // Neuron selected by NID channel 1
    N2          =   0b1010, // Neuron selected by NID channel 2
    N3          =   0b1100, // Neuron selected by NID channel 3
    N4          =   0b1011, // Neuron selected by NID channel 4
    SELECTED    =   0b1110, // Neuron with 'select' button pressed
    UPSTREAM    =   0b1101  // Neuron communicating to downstream neuron from axon->dendrite (only messages from axon get this identifier)
} sender_identifiers;

typedef enum{
    KEEP_ALIVE  =   0b000001,
    PULSE       =   0b000010,
    BLINK       =   0b000100
} message_identifiers;

extern uint16_t active_input_pins[11];

extern uint32_t active_input_ports[11];

uint8_t message_buffer_count[11];
uint32_t message_buffer[11];

uint32_t messages[4];
uint8_t messages_count;

void readInputs(void); // The readInputs() function reads incoming messages and decides if it the data frame should be sent to a handler based off of sender id and message id.

uint16_t active_output_pins[11];

uint32_t active_output_ports[11];

// write outputs from message buffer
extern uint32_t write_buffer[3]; // buffer for messages that will be sent through all outputs except for the original sender
extern uint8_t write_buffer_ready;

extern uint32_t downstream_write_buffer; // buffer just for messages traveling downstream through axon
extern uint8_t downstream_write_buffer_ready;

extern uint32_t nid_write_buffer[3]; // buffer just for messages being sent to NID
extern uint8_t nid_write_buffer_ready;

extern uint32_t *current_write_buffer;
extern uint8_t write_count; // Incremented after each bit is written. Write new message after 32-bits.

void writeOutputs(void);
void writeDownstream(void);
void writeNID(void);

// received message handlers
void receivePulse(uint32_t message);
void receiveUpstreamKeepAlive(uint32_t message, uint16_t pin);
void receiveUpstreamPulse(uint32_t message, uint16_t pin);
void receiveNIDKeepAlive(uint32_t message, uint16_t pin);
void receiveNIDBlink(void);
void initializeVars(void);

// send message handlers
//void sendDownstream(uint16_t data);
//void passMessage(uint32_t message);


#endif