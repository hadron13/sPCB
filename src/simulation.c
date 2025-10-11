
#include"circuit_data.h"
#include"list.h"

typedef struct{


}simulation_data;



void rom_init(component_t *component){

    component->data = calloc(16, 1); 
    component->pins = calloc(6, sizeof(pin_t));


    component->pins[0].type = SUPPLY;
    component->pins[1].type = IN; //mosi
    component->pins[2].type = IN; //clock
    component->pins[3].type = OUT;//miso
    component->pins[4].type = IN; //CE
    component->pins[5].type = GROUND;
    

}


circuit_t test_circuit;

void simulation_init(){
    test_circuit.components = list_init(component_t);
    

    
}

void simulation_step(){

}

