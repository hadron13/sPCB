#include"circuit_data.h"
#include<stdbool.h>
#include<stdlib.h>



component_t ti7404_init(){
    component_t component;

    component.pins = calloc(14, sizeof(pin_t));
    pin_t *pins = component.pins;

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

    return component;
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

component_t ti7400_init(){
    component_t component;

    component.pins = calloc(14, sizeof(pin_t));
    pin_t *pins = component.pins;

    pins[0].type = IN;     pins[13].type = SUPPLY;
    pins[1].type = IN;     pins[12].type = IN; 
    pins[2].type = OUT;    pins[11].type = IN; 
    pins[3].type = IN;     pins[10].type = OUT; 
    pins[4].type = IN;     pins[9].type  = IN; 
    pins[5].type = OUT;    pins[8].type  = IN;  
    pins[6].type = GROUND; pins[7].type  = OUT; 
    
    return component;
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


