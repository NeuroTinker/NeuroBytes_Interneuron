
#include "neuron.h"
#include "comm.h"

uint16_t input_pins[11] = {
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

void neuronInit(neuron_t *n)
{
	uint8_t i;

	n->potential = 0;
	n->state = INTEGRATE;

	n->fire_time = 0;
	n->fire_potential = 0;

	n->hebb_time = 0;
	n->learning_state = NONE;

	for (i=0;i<DENDRITE_COUNT;i++){
		n->dendrites[i].state = OFF;
		n->dendrites[i].current_value = 0;
		n->dendrites[i].type = EXCITATORY;
		n->dendrites[i].timestamp = 0;
		n->dendrites[i].pulse_time = 0;
		n->dendrites[i].alive_time = 0;
	}

	n->dendrites[0].magnitude = 100;
	n->dendrites[1].magnitude = 100;
	n->dendrites[2].magnitude = 100;
	n->dendrites[3].magnitude = 100;
	n->dendrites[4].magnitude = 100;
	
	n->dendrite_ping_time[0] = 0;
	n->dendrite_ping_time[1] = 0;
	n->dendrite_ping_time[2] = 0;
	n->dendrite_ping_time[3] = 0;
	n->dendrite_ping_time[4] = 0;
	n->dendrite_ping_time[5] = 0;
	n->dendrite_ping_time[6] = 0;
	n->dendrite_ping_time[7] = 0;
	n->dendrite_ping_time[8] = 0;
	n->dendrite_ping_time[9] = 0;
	n->dendrite_ping_time[10] = 0;
}

void checkDendrites(neuron_t * n)
{
	uint8_t i;
	dendrite_states current_state = OFF;
	
	for (i=1; i<11; i++){
		
		if (dendrite_ping_flag[i] != 0){

			dendrite_ping_flag[i] = 0;

			n->dendrite_ping_time[i] = DEND_PING_TIME;
		} else if (n->dendrite_ping_time[i] == 1){
			//setAsInput(active_input_ports[i], input_pins[i]);
			
			if (i % 2 != 0){
				setAsInput(active_input_ports[i+1], complimentary_pins[i]);
				active_output_pins[i+1] = 0;
			} else{
				setAsInput(active_input_ports[i+1], complimentary_pins[i]);
				active_output_pins[i-1] = 0;
			}
			
		}
		
		if (n->dendrite_ping_time[i] > 0){
			n->dendrite_ping_time[i] -= 1;
		}
		
		if (dendrite_pulse_flag[i] != 0){
			dendrite_pulse_flag[i] = 0;

			switch (input_pins[i]){
				case PIN_DEND1_EX:
					n->dendrites[0].type = EXCITATORY;
					n->dendrites[0].state = ON;
					n->dendrites[0].pulse_time = 0;
					break;
				case PIN_DEND1_IN:
					n->dendrites[0].type = INHIBITORY;
					n->dendrites[0].state = ON;
					n->dendrites[0].pulse_time = 0;
					break;
				case PIN_DEND2_EX:
					n->dendrites[1].type = EXCITATORY;
					n->dendrites[1].state = ON;
					n->dendrites[1].pulse_time = 0;
					break;
				case PIN_DEND2_IN:
					n->dendrites[1].type = INHIBITORY;
					n->dendrites[1].state = ON;
					n->dendrites[1].pulse_time = 0;
					break;
				/*
				case PIN_DEND3_EX:
					n->dendrites[2].type = EXCITATORY;
					n->dendrites[2].state = ON;
					n->dendrites[2].pulse_time = 0;
					break;
				case PIN_DEND3_IN:
					n->dendrites[2].type = INHIBITORY;
					n->dendrites[2].state = ON;
					n->dendrites[2].pulse_time = 0;
					break;
				case PIN_DEND4_EX:
					n->dendrites[3].type = EXCITATORY;
					n->dendrites[3].state = ON;
					n->dendrites[3].pulse_time = 0;
					break;
				case PIN_DEND4_IN:
					n->dendrites[3].type = INHIBITORY;
					n->dendrites[3].state = ON;
					n->dendrites[3].pulse_time = 0;
					break;
				*/
				case PIN_DEND5_EX:
					n->dendrites[4].type = EXCITATORY;
					n->dendrites[4].state = ON;
					n->dendrites[4].pulse_time = 0;
					break;
				case PIN_DEND5_IN:
					n->dendrites[4].type = INHIBITORY;
					n->dendrites[4].state = ON;
					n->dendrites[4].pulse_time = 0;
					break;
				default:
					break;
			}
		}
	}
	
	for (i=0; i<DENDRITE_COUNT; i++){

		if(n->dendrites[i].state == ON){
			n->dendrites[i].pulse_time += 1;
			if (n->dendrites[i].pulse_time >= PULSE_LENGTH){
				dendriteSwitchOff(&(n->dendrites[i]));
			}
		}
		
		// n->dendrites[i].timestamp++;
	}
	
}

void incrementHebbTime(neuron_t * n)
{
	uint8_t i;

	n->ms_count++;
	if (n->ms_count == n->time_multiple){
		n->hebb_time++;
		n->ms_count = 0;
	}

	if (n->hebb_time == UINT16_MAX - 1){
		n->hebb_time /= 2;
		for (i=0; i<DENDRITE_COUNT; i++){
			n->dendrites[i].timestamp /= 2;
		}
		n->time_multiple *= 2;
	}
}

void dendriteSwitchOff(dendrite_t *dendrite)
{
	dendrite->state = OFF;
	dendrite->pulse_time = 0;
	
	switch(dendrite->type){
		case EXCITATORY:
			dendrite->current_value += dendrite->magnitude;
			break;
		case INHIBITORY:
			dendrite->current_value -= dendrite->magnitude;
			break;
	}
}

void dendriteDecayStep(neuron_t * n)
{
	uint8_t i;

	for(i=0; i<DENDRITE_COUNT; i++){
		n->dendrites[i].current_value = (n->dendrites[i].current_value * 63 ) / 64;
	}
}

void membraneDecayStep(neuron_t * n)
{
	n->fire_potential = (n->fire_potential * 63) / 64;
}

int16_t calcNeuronPotential(neuron_t *n)
{
	uint8_t i;
	int16_t new_v = 0;
	for (i=0; i<DENDRITE_COUNT; i++){
		if (n->dendrites[i].state == ON){
			switch(n->dendrites[i].type){
				case EXCITATORY:
					new_v += n->dendrites[i].magnitude;
					break;
				case INHIBITORY:
					new_v -= n->dendrites[i].magnitude;
					break;
			}
		}
		new_v += n->dendrites[i].current_value; // each dendrite contributes its decay (*.current_value) and magnitude
	}
	return new_v;
}
