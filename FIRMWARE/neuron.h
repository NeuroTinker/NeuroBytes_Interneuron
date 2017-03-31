#ifndef NEURON_H_
#define NEURON_H_

#define MEMBRANE_THRESHOLD      120
#define HYPERPOLARIZATION		-200
#define DENDRITE_COUNT          5

typedef enum{
NC =   0,
CONNECTED =    1
} dendrite_status;

typedef enum{
OFF =   0,
ON =    1
} dendrite_states;

typedef enum{
EXCITATORY = 0,
INHIBITORY = 1
} dendrite_types;

typedef struct{
gpio_port          port;
gpio_pin           pin;
dendrite_states     state;
dendrite_status     status;
dendrite_types      type;
int16_t             magnitude; // weighting
int16_t             current_value;
uint8_t             nid_flag; // is this dendrite the closest to the NID
uint8_t             read_flag; // dendrite is currently getting a message
uint8_t             read_time; // message length decrementor
uint8_t             history;
uint16_t			timestamp; // last input pulse (used for learning)
} dendrite_t;

typedef enum{
INTEGRATE =       0,
FIRE =          1,
REST =			2 // izhik
} neuron_states;

typedef enum{
NONE =	0,
HEBB =	1,
SLEEP =	2
} learning_states;

typedef struct{
// mode independent vars
int16_t     potential;
dendrite_t  dendrites[DENDRITE_COUNT];
neuron_states   state;
learning_states learning_state; // Hebb learning mode

uint16_t    fire_time;
int16_t		fire_potential;

// Hebb vars
uint16_t	hebb_time;
uint8_t		time_multiple;
uint16_t	ms_count;

} neuron_t;

void neuronInit(neuron_t *n);
void checkDendrites(neuron_t * n);
int16_t calcNeuronPotential(neuron_t *n);
void dendriteDecayStep(dendrite_t * dendrites);
void membraneDecayStep(neuron_t * n);
void dendriteSwitchOff(dendrite_t * dendrite);
void incrementHebbTime(neuron_t *n);

int16_t dendrite_magnitude_lookup[5];

#endif