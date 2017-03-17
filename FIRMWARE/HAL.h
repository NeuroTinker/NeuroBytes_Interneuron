#ifndef HAL_H_
#define HAL_H_

/* NeuroBytes v1.01 
#define PORT_LED		GPIOA
#define PIN_R_LED		GPIO2
#define PIN_G_LED		GPIO5
#define PIN_B_LED		GPIO3

#define PORT_AXON		GPIOA
#define PIN_AXON_EX		GPIO10
#define PIN_AXON_IN		GPIO9

#define PORT_IDENTIFY	GPIOA
#define PIN_IDENTIFY	GPIO8
*/

/* Motor Neuron */
#define PORT_LED		GPIOA
#define PIN_R_LED		GPIO0
#define PIN_G_LED		GPIO2
#define PIN_B_LED		GPIO1


extern volatile uint8_t tick;
static const uint16_t gamma_lookup[1024];

void systick_setup(int freq);
void clock_setup(void);
void gpio_setup(void);
void tim_setup(void);
void LEDfullWhite(void);
void setLED(uint16_t r, uint16_t g, uint16_t b);

//extern volatile unsigned char ms_tick;

#endif
