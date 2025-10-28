
#include"circuit_data.h"
#include"list.h"
#include<SDL3/SDL_log.h>

typedef struct{


}simulation_data;



void ti7404_init(component_t *component){

    component->type = 1;

    component->pins = calloc(14, sizeof(pin_t));
    pin_t *pins = component->pins;

    pins[0].type = IN; 
    pins[1].type = OUT; 
    pins[2].type = IN; 
    pins[3].type = OUT; 
    pins[4].type = IN; 
    pins[5].type = OUT; 
    pins[6].type = GROUND;

    pins[7].type = OUT; 
    pins[8].type = IN; 
    pins[9].type = OUT; 
    pins[10].type = IN; 
    pins[11].type = OUT; 
    pins[12].type = IN; 
    pins[13].type = SUPPLY;
}

void ti7404_update(component_t *component, double dt){
    float supply = component->pins[13].voltage;
    float ground = component->pins[6].voltage;
    pin_t *pins = component->pins;

    for(int i = 0; i < 13; i+=2){
        if(i == 6) i++;

        pin_t *input_pin  = &pins[i < 7? i   : i+1];
        pin_t *output_pin = &pins[i < 7? i+1 : i  ];

        bool a = input_pin->voltage > 2.0;
        output_pin->mode        = a? DRAIN  :SOURCE;
        output_pin->voltage     = a? ground :supply;    
        output_pin->max_current = a? 8 * mA :400 * uA;    
    }
}

void ti7400_init(component_t *component){
    component->type = 1;

    component->pins = calloc(14, sizeof(pin_t));
    pin_t *pins = component->pins;

    pins[0].type = IN;     pins[13].type = SUPPLY;
    pins[1].type = IN;     pins[12].type = IN; 
    pins[2].type = OUT;    pins[11].type = IN; 
    pins[3].type = IN;     pins[10].type = OUT; 
    pins[4].type = IN;     pins[9].type  = IN; 
    pins[5].type = OUT;    pins[8].type  = IN;  
    pins[6].type = GROUND; pins[7].type  = OUT; 
    
}

void ti7400_update(component_t *component, double dt){
    float supply = component->pins[13].voltage;
    float ground = component->pins[6].voltage;
    pin_t *pins = component->pins;

    for(int i = 0; i < 13; i+=3){
        if(i == 6) i++;

        pin_t *input_pin_a  = &pins[i < 7? i   : i+1];
        pin_t *input_pin_b  = &pins[i < 7? i+1 : i+1];
        pin_t *output_pin   = &pins[i < 7? i+2 : i  ];

        bool a = input_pin_a->voltage > 2.0;
        bool b = input_pin_b->voltage > 2.0;

        output_pin->mode        = (a && b)?DRAIN:SOURCE;
        output_pin->voltage     = (a && b)?ground:supply;    
        output_pin->max_current = (a && b)? 8 * mA :400 * uA;    
    }
}


void dump_chip(component_t *component, int pins){
    SDL_Log("----------");
    for(int i = 0; i < pins/2; i++){
        SDL_Log("%.2f \t%i --- %i \t%.2f", component->pins[i].voltage, i+1, pins-i, component->pins[i+pins/2].voltage);
    }
}


circuit_t test_circuit;

void simulation_init(){
    test_circuit.components = calloc(10, sizeof(component_t));
    test_circuit.nodes      = calloc(10, sizeof(node_t));

    ti7400_init(&test_circuit.components[0]); 
    ti7404_init(&test_circuit.components[1]); 

    test_circuit.components[0].pins[13].voltage = 5.0;
    test_circuit.components[1].pins[13].voltage = 4.5;
    test_circuit.components[1].pins[0].voltage = 5.0;

    test_circuit.nodes[0] = (node_t){
        .voltage = 0.0,
        .pins = calloc(10, sizeof(pin_t*)),
        .n_pins = 3
    };
    test_circuit.nodes[0].pins[0] = &test_circuit.components[0].pins[0];
    test_circuit.nodes[0].pins[1] = &test_circuit.components[0].pins[1];
    test_circuit.nodes[0].pins[2] = &test_circuit.components[1].pins[1];

}



void simulation_step(){
    ti7400_update(&test_circuit.components[0], 1.0/1000);
    ti7404_update(&test_circuit.components[1], 1.0/1000);
    
    node_t *node = &test_circuit.nodes[0];
    
    for(int i = 0; i < node->n_pins; i++){
        pin_t *pin = node->pins[i];
        SDL_Log("%i", pin->type);
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
    
    SDL_Log("frame ---");
    SDL_Log("7400");
    dump_chip(&test_circuit.components[0], 14); 
    SDL_Log("7404");
    dump_chip(&test_circuit.components[1], 14); 
}

