#pragma once

#include<stdint.h>

typedef enum{
    PASSIVE, IN, OUT, INOUT, SUPPLY, GROUND
}pin_type_t;

typedef enum{
    NONE, SOURCE, DRAIN, OPEN, PULLUP, PULLDOWN, HIGH_IMPEDANCE
}pin_mode_t;

typedef struct{
    pin_type_t type;
    pin_mode_t mode;
    float max_current;
    float pull_voltage;
    float pull_resistance;
    float voltage;
}pin_t;

typedef struct{
    uint32_t type;
    void  *data;
    pin_t *pins;
}component_t;

typedef struct{
    void(*init)(component_t *);
    void(*update)(component_t *, float dt);
}component_model_t;

typedef struct{
    pin_t *pins;
}node_t;

typedef struct{
    component_t *components;
}circuit_t;


