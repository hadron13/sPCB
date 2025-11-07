
#include"circuit_data.h"
#include"list.h"
#include"simulation.h"
#include<SDL3/SDL_log.h>
#include<stdint.h>



component_t ti7404_init();
component_t ti7400_init();

void ti7404_update(component_t *component, double dt);
void ti7400_update(component_t *component, double dt);

void dump_chip(component_t *component, int pins){
    SDL_Log("----------");
    for(int i = 0; i < pins/2; i++){
        SDL_Log("%.2f \t%i --- %i \t%.2f", component->pins[i].voltage, i+1, pins-i, component->pins[i+pins/2].voltage);
    }
}

simulation_data_t sim;
component_model_t models[10];





void simulation_init(){
    sim.circuit.components = calloc(10, sizeof(component_t));
    sim.circuit.nodes      = calloc(10, sizeof(node_t));

    models[0] = (component_model_t){
        .name = "7400",
        .description = "Quad NAND gate",
        .init = ti7400_init,
        .update = ti7400_update,
        .n_pins = 14
    };
    models[1] = (component_model_t){
        .name = "7404",
        .description = "Hex inverter",
        .init = ti7404_init,
        .update = ti7404_update,
        .n_pins = 14
    };
    sim.circuit.components[0].type = 0;
    sim.circuit.components[1].type = 1;


    for(int i = 0; i < 2; i++){
        int id = sim.circuit.components[i].type;
        sim.circuit.components[i] = models[id].init();
        sim.circuit.components[i].type = id;
    }


    sim.circuit.components[0].pins[13].voltage = 5.0;
    sim.circuit.components[1].pins[13].voltage = 4.5;
    // sim.circuit.components[1].pins[0].voltage = 5.0;

    sim.circuit.nodes[0] = (node_t){
        .voltage = 0.0,
        .pins = calloc(10, sizeof(pin_t*)),
        .n_pins = 3
    };
    sim.circuit.nodes[0].pins[0] = &sim.circuit.components[0].pins[0];
    sim.circuit.nodes[0].pins[1] = &sim.circuit.components[0].pins[1];
    sim.circuit.nodes[0].pins[2] = &sim.circuit.components[1].pins[1];

}



void simulation_step(){
    
    for(int i = 0; i < 2; i++){
        int id = sim.circuit.components[i].type;
        models[id].update(&sim.circuit.components[i], 1.0/1000);
    }

    node_t *node = &sim.circuit.nodes[0];
    
    for(int i = 0; i < node->n_pins; i++){
        pin_t *pin = node->pins[i];
        if(pin->type == OUT){
            node->voltage = pin->voltage;
            break;
        }
    }
    for(int i = 0; i < node->n_pins; i++){
        pin_t *pin = node->pins[i];
        if(pin->type == IN){
            pin->voltage = node->voltage;
        }
    }
    

}

circuit_t *simulation_circuit(){
    return &sim.circuit;
}
