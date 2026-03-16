#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<stddef.h>
#include<stdint.h>


typedef enum{
    UNDEFINED, 
    INPUT,
    OUTPUT,
    BIDIRECTIONAL, 
    TRI_STATE, 
    PASSIVE, 
    FREE, 
    UNSPECIFIED, 
    POWER_IN, 
    POWER_OUT, 
    OPEN_COLLECTOR, 
    OPEN_EMITTER, 
    NC
}pin_type_t;

typedef enum{
    PINSTYLE_LINE, 
    PINSTYLE_INVERTED,
    PINSTYLE_CLOCK,
    PINSTYLE_INVERTED_CLOCK,
    PINSTYLE_INPUT_LOW,
    PINSTYLE_CLOCK_LOW,
    PINSTYLE_OUTPUT_LOW,
    PINSTYLE_EDGE_CLOCK_HIGH,
    PINSTYLE_NON_LOGIC
}pin_style_t;

typedef struct{
    float x, y;
}point_t;

typedef enum{
    DRAW_ARC, DRAW_CIRCLE, DRAW_CURVE, DRAW_LINE, DRAW_RECTANGLE, DRAW_TEXT, DRAW_PIN
}draw_command_type;

typedef struct{
    draw_command_type type;
    union{
        struct{
            point_t start, mid, end;    
        }arc;
        struct{
            point_t center;
            float radius;
        }circle;
        struct{
            uint32_t n_points;
            point_t *points;
        }curve;
        struct{
            point_t start, end;
        }line;
        struct{
            point_t start, end;
        }rect;
        struct{
            point_t start, end;
        }pin;
        char *text;
    }data;
}draw_command_t;

typedef struct{
    uint64_t model_id;
    float x, y, angle;
    void *state;
    uint16_t n_pins;
    float *pin_voltages;
    pin_type_t *pin_modes;
}component_t;

typedef struct{
    uint64_t id;
    char *name;
    uint16_t n_pins;
    void(*create)(component_t *);
    void(*update)(component_t *, double dt);
    draw_command_t *render_data;
}component_model_t;

typedef struct{
    int a;
}network_t;

typedef struct{
    component_t *components;

}circuit_t;







