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
    DRAW_ARC, DRAW_CIRCLE, DRAW_BEZIER, DRAW_LINE, DRAW_RECTANGLE, DRAW_TEXT, DRAW_PIN
}shape_type_t;

typedef enum{
    STYLE_SOLID, STYLE_DASHED, STYLE_DOTTED 
}line_style_t;

//generic graphical component
typedef struct{
    shape_type_t type;
    struct{
        uint32_t color;
        uint32_t fill_color;
        float line_width;
        line_style_t line_style;
        bool fill;
        float font_size;
    }stroke;
    union{
        struct{
            point_t center;
            point_t start, middle, end;
            float radius, start_angle, end_angle, offset_angle, length;
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
        } bezier;
        struct{
            pin_style_t style;
            point_t start, end;
        }pin;
        struct{
            char *string;
            point_t position;
        }text;
    }data;
}shape_t;




typedef struct{
    void *state;
    float *pin_voltages;
}component_t;

//KiCad Unit
typedef struct{
    int unit;
    int style;
    shape_t *graphics;
    shape_t *pins;
}unit_type_t;

//KiCad Symbol
typedef struct{
    char *name;
    unit_type_t *units;
    bool has_common_units;
}symbol_type_t;

typedef struct{
    size_t lib_index;
    int unit;
    int style;
    point_t  position;
    int rotation;
    shape_t *properties;
    unit_type_t *cached_common_unit;
    unit_type_t *cached_unit;
}symbol_t;

typedef struct{
    float *pins; 
}net_t;

//unified schematic data 
//based on KiCad schematic file format
typedef struct{
    uint32_t version;
    char *path;
    
    symbol_type_t *symbol_library;
    symbol_t *symbols;

    shape_t *wires;
    shape_t *junctions;
}circuit_t;







