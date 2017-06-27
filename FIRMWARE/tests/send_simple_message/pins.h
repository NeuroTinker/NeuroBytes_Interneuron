#define PORT_R_LED      GPIOB
#define PORT_G_LED      GPIOA
#define PORT_B_LED      GPIOB
#define PIN_R_LED		GPIO7
#define PIN_G_LED		GPIO1
#define PIN_B_LED		GPIO6

#define PORT_AXON1_EX   GPIOA
#define PORT_AXON1_IN   GPIOC

#define PORT_DEND1_EX   GPIOB
#define PORT_DEND1_IN   GPIOB

#define PIN_AXON1_EX    GPIO8
#define PIN_AXON1_IN    GPIO15

#define PIN_DEND1_EX    GPIO1
#define PIN_DEND1_IN    GPIO0

#define DEND1_EX        all_pins[2]
#define DEND1_IN        all_pins[3]
#define AXON1_EX        all_pins[0]
#define AXON1_IN        all_pins[1]

#define NUM_INPUTS      3
#define NUM_PINS        4
#define NUM_OUTPUTS     3

typedef struct {
    uint32_t pin;
    uint32_t port;
} pin_address_t;

typedef enum {
    INHIB,
    EXCIT
} pin_type_t;

typedef enum {
    DEND,
    AXON
} port_type_t;

typedef enum {
    INPUT,
    OUTPUT
} pin_state_t;

typedef enum{
    MESSAGE,
    DATA
} input_state_t;

struct input_buffer_t{
    uint64_t message;
    uint8_t read_tick;
    uint8_t num_bits_to_read;
    input_state_t state;
};

typedef struct {
    uint64_t message;
    uint8_t bits_left_to_write;
    uint8_t num_output_pins;
    pin_t * output_pins[NUM_OUTPUTS];
} output_buffer_t;

struct pin_t{
    pin_address_t * address;
    pin_type_t pin_type;
    port_type_t port_type;
    pin_t * complimentary_pin_ptr;
    pin_state_t io_state;
    uint32_t exti;
    bool active;
    //dendrite_t * dendrite_ptr;
    input_buffer_t * input_buffer;
};

typedef enum{
    ALL,
    NID,
    AXONS
} pin_group_name_t;

typedef struct {
    pin_group_name
}

