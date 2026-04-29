#pragma once
#include<stdio.h>
#include<stdlib.h>
#include<stddef.h>
#include<stdint.h>
#include<stdbool.h>

#include"hashmap.h"

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
    DRAW_ARC, DRAW_CIRCLE, DRAW_POLYGON, DRAW_BEZIER, DRAW_LINE, DRAW_RECTANGLE, DRAW_TEXT, DRAW_PIN
}shape_type_t;

typedef enum{
    STYLE_SOLID, STYLE_DASHED, STYLE_DOTTED 
}line_style_t;

typedef struct{
    shape_type_t type;
    struct{
        uint32_t color;
        uint32_t fill_color;
        float line_width;
        line_style_t line_style;
        bool fill;
    }stroke;
    union{
        struct{
            point_t start, mid, end;    
        }arc;
        struct{
            point_t center;
            float radius;
        }circle;
        struct{
            point_t start, end;
        }line;
        struct{
            point_t start, end;
        }rect;
        struct{
            uint32_t n_points;
            point_t *points;
        }polygon, bezier;
        struct{
            pin_style_t style;
            point_t start, end;
        }pin;
        char *text;
    }data;
}shape_t;


/*
typedef struct{
    uint64_t model_id;
    float x, y, angle;
    void *state;
    uint16_t n_pins;
    float *pin_voltages;
    pin_type_t *pin_modes;
}component_t;

typedef struct{
    uint16_t number;
    pin_style_t style;
    pin_type_t type;
}schematic_pin_t;

typedef struct{
    point_t          placement;
    shape_t  *shapes;
    schematic_pin_t *pins;
}schematic_component_t;

typedef struct{
    uint64_t id;
    char *name;
    uint16_t n_pins;
    void(*create)(component_t *);
    void(*update)(component_t *, double dt);
}logical_component_model_t;


typedef struct{
    logical_component_model_t  *model;
    schematic_component_t *schematic_components;
    void  *state;
    float *pin_voltages;
    float *pin_currents;
}logical_component_t;

typedef struct{
    logical_component_model_t *logical_models;
    logical_component_t *logical_components;
}schematic_t;
=======
    shape_t *render_data;
}component_model_t;
*/

typedef struct{
    uint32_t symbol_type_id;
    uint32_t component_id;
    point_t  position;
    float    rotation;
}symbol_t;

typedef struct{
    void *state;
    float *pin_voltages;
}component_t;

typedef struct{
    uint32_t net_id;
    shape_t *draw_commands;
}wire_t;

typedef struct{
    symbol_t *symbols;
    wire_t *wires;
}schematic_t;

//KiCad 'sub'symbol
typedef struct{
    shape_t *draw_commands;
}symbol_type_t;

//KiCad symbol
typedef struct{
    char *name;
    symbol_type_t *symbol_types;
}component_type_t;

typedef struct{
    float *pins; 
}net_t;

//unified schematic data 
//based on KiCad schematic file format
typedef struct{
    uint32_t version;
    char *path;
    
    component_type_t *symbol_library;
    schematic_t *schematics;
    component_t *components;

    shape_t *wires;
}circuit_t;







