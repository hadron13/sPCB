#include"parser.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_log.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <wchar.h>
#include "../list.h"
#include "cglm/vec2.h"
#include "cglm/vec3.h"
#include "src/data.h"
#include <math.h>


point_t point_sub(point_t a, point_t b){
    return (point_t){a.x - b.x, a.y - b.y};
}

point_t find_arc_center(point_t start, point_t mid, point_t end) {
    point_t center;
    float d1 = glm_vec2_dot(&start.x, &start.x);
    float d2 = glm_vec2_dot(&mid.x, &mid.x);
    float d3 = glm_vec2_dot(&end.x, &end.x);

    float D = 2.0f * (start.x * (mid.y - end.y) + 
                      mid.x * (end.y - start.y) + 
                      end.x * (start.y - mid.y));

    if (fabsf(D) < 1e-6f) {
        return (point_t){0, 0}; 
    }

    center.x = (d1 * (mid.y - end.y) + 
                 d2 * (end.y - start.y) + 
                 d3 * (start.y - mid.y)) / D;

    center.y = (d1 * (end.x - mid.x) + 
                 d2 * (start.x - end.x) + 
                 d3 * (mid.x - start.x)) / D;
    return center;
}
    


typedef enum{
    VALUE_NUMBER, VALUE_BOOLEAN, VALUE_STRING, VALUE_GROUP
}parse_value_type_t;

typedef struct parse_value{
    parse_value_type_t type;
    union{
        double number;
        bool   boolean;
        char  *string; 
        struct{
            char *identifier; 
            struct parse_value *children;
        };
    };
}parse_value_t;


typedef struct{
    char *p;
}parse_context_t;


char *str_dup(char *str){
    size_t len = strlen(str);
    char *buf = malloc(len + 1);
    memcpy(buf, str, len);
    buf[len] = '\0';
    return buf;
}

char *str_dup1(char *str){
    size_t len = strlen(str);
    char *buf = malloc(len);
    memcpy(buf, str, len - 1);
    buf[len - 1] = '\0';
    return buf;
}

size_t token_size(char *str){
    char *p = str;
    if(*p == '"'){
        p++;
        while(*p != '"' && *p != '\0')p++;
        return p - str + 1;
    }else{
        while(!isspace(*p) && *p != ')' && *p != '\0') p++; 
    }
    return p - str;
}


char *copy_token_advance(volatile parse_context_t *ctx){

    size_t len = token_size(ctx->p);
    
    char *token = malloc(len + 1);
    memcpy(token, ctx->p, len);    
    token[len] = '\0';
    ctx->p += len;
    
    return token;
}

bool debug_print_val = false;

parse_value_t parse_expression(parse_context_t *ctx){
   
    parse_value_t value;

    while(isspace(*ctx->p)) ctx->p++; 

    if(*ctx->p == '('){
        ctx->p++;
       
        value.type = VALUE_GROUP;
        value.identifier = copy_token_advance(ctx);
        value.children = list_init(parse_value_t);

        while(*ctx->p != ')' && *ctx->p != '\0'){
            //if(strcmp(value.identifier, "lib_symbols") == 0){
                // debug_print_val = true;
            //}
            parse_value_t child_value = parse_expression(ctx);
            list_push(value.children, child_value);
            while(isspace(*ctx->p)) ctx->p++; 
        }
        ctx->p++; 
        
    }else{ 
        char *token = copy_token_advance(ctx);

        if(strlen(token) == 0){
            // ?????
            value.string = NULL;
            SDL_Log("Unexpected 0 length token, likely an error");

        }else if(token[0] == '"'){

            value.type = VALUE_STRING;
            value.string = str_dup1(token + 1);

        }else if(isdigit(token[0]) || (token[0] == '-' && isdigit(token[1]))){
            
            value.type = VALUE_NUMBER;
            value.number = atof(token);

        }else{
            if(strcmp(token, "yes") == 0){
                value.type = VALUE_BOOLEAN;
                value.boolean = true;
            }else if(strcmp(token, "no") == 0){
                value.type = VALUE_BOOLEAN;
                value.boolean = false; 
            }else{
                value.type = VALUE_STRING;
                value.string = str_dup(token);
            }
        }
        if(debug_print_val){
            SDL_Log("%s", token);
            debug_print_val = false;
        }


        free(token);
    }


    return value;
}

void free_expression(parse_value_t value){
    switch(value.type){
        case VALUE_STRING:
            free(value.string);
            break;
        case VALUE_GROUP:
            free(value.identifier);
            for(int i = 0; i < list_size(value.children); i++){
                free_expression(value.children[i]);
            }
            list_free(value.children);
            break;
        default:
            break;
    }
}


static char *load_file(const char *path){
    FILE *file = fopen(path, "rb");

    if (file == NULL){
        // printf("error opening file %s", path);
        perror("error");
        return NULL;
    }
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);  

    char *string = malloc(file_size + 1);
    if(fread(string, file_size, 1, file) == 0){
        return NULL;
    }

    string[file_size] = 0;
    fclose(file);
    
    return string;
}

void debug_print_parse_value_recursive(parse_value_t value){
    SDL_Log("type: %s", value.type == VALUE_GROUP? "group": value.type == VALUE_NUMBER? "number": value.type == VALUE_STRING? "string" : "boolean");
    switch(value.type){
        case VALUE_GROUP: 
            SDL_Log("id: %s - children: %zu", value.identifier, list_size(value.children)); 
            for(int i = 0; i < list_size(value.children); i++){
                debug_print_parse_value_recursive(value.children[i]);
            }
            break;
        case VALUE_BOOLEAN: SDL_Log("%s", value.boolean?"true":"false"); break;
        case VALUE_NUMBER: SDL_Log("%lf", value.number); break;
        case VALUE_STRING: SDL_Log("%p", value.string);break;
    }
}
void debug_print_parse_value(parse_value_t value){
    SDL_Log("type: %s", value.type == VALUE_GROUP? "group": value.type == VALUE_NUMBER? "number": value.type == VALUE_STRING? "string" : "boolean");
    switch(value.type){
        case VALUE_GROUP: SDL_Log("id: %s - children: %zu", value.identifier, list_size(value.children)); break;
        case VALUE_BOOLEAN: SDL_Log("%s", value.boolean?"true":"false"); break;
        case VALUE_NUMBER: SDL_Log("%lf", value.number); break;
        case VALUE_STRING: SDL_Log("%p", value.string);break;
    }
}


// (whatever X Y) format
point_t parse_position(parse_value_t parse_value){
    if(list_size(parse_value.children) < 2)
        return (point_t){0, 0};

    return (point_t){
        parse_value.children[0].number,
        parse_value.children[1].number,
    };
}

point_t flip_y(point_t point){
    return (point_t){point.x, -point.y};
}
point_t flip_x(point_t point){
    return (point_t){-point.x, point.y};
}
point_t flip_xy(point_t point){
    return (point_t){-point.x, -point.y};
}

void add_symbol_type_to_library(parse_value_t parse_value, circuit_t *circuit);
void add_unit_to_symbol_type(parse_value_t parse_value, symbol_type_t *type);
void add_wire(parse_value_t parse_value, circuit_t *circuit);
void add_shape_to_list(parse_value_t parse_value, shape_t **list);


circuit_t parse_schematic(char *path){
    
    circuit_t circuit;
    char *file = load_file(path);

    if(file == NULL){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Could not load file at path %s", path);
        return circuit;
    }
        

    SDL_Log("Parsing file %s ...", path);
    uint64_t start_time = SDL_GetPerformanceCounter();

    parse_context_t ctx = {file};
    parse_value_t parsed_output = parse_expression(&ctx);

    uint64_t time_spent = SDL_GetPerformanceCounter() - start_time;  
    SDL_Log("Parsed file in %f seconds", (double)time_spent / SDL_GetPerformanceFrequency());

    free(file);

    
    if(strcmp(parsed_output.identifier, "kicad_sch") != 0){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Expected schematic as input!");
        return circuit;
    }
    
    SDL_Log("Interpreting file parse output...");

    circuit = (circuit_t){
        .path = str_dup(path),
        .symbol_library = list_init(symbol_type_t),
        .symbols = list_init(symbol_t),
        .wires = list_init(shape_t),
        .junctions = list_init(shape_t),
    };


    for(int i = 0; i < list_size(parsed_output.children); i++){
        parse_value_t val = parsed_output.children[i];
        
        if(strcmp(val.identifier, "version") == 0){
            circuit.version = (uint32_t)val.children[0].number;
            SDL_Log("file version %u", (uint32_t)val.children[0].number);
            continue;
        }
        if(strcmp(val.identifier, "lib_symbols") == 0){
            SDL_Log("%zu component types in library", list_size(val.children));
            for(int j = 0; j < list_size(val.children); j++){
                // debug_print_parse_value(val.children[j]);
                //SDL_Log("%s", val.children[j].identifier);
                

                if(val.children[j].type != VALUE_GROUP || strcmp(val.children[j].identifier, "symbol"))
                    continue;
                

                add_symbol_type_to_library(val.children[j], &circuit);
            }
            continue;
        }
        if(strcmp(val.identifier, "wire") == 0){
            // debug_print_parse_value_recursive(val);
            //
            
            parse_value_t pts = val.children[0];

            shape_t wire_shape = {
                .type = DRAW_LINE,
                .stroke = {
                    .color = 0x00FF00FF,
                    .line_width = 0.25f 
                },
                .data.line = {
                    .start = parse_position(pts.children[0]),
                    .end = parse_position(pts.children[1])
                }
            };
            list_push(circuit.wires, wire_shape);
            continue;
        }
        if(strcmp(val.identifier, "junction") == 0){
            // debug_print_parse_value_recursive(val);
            shape_t junction_shape = {
                .type = DRAW_CIRCLE,
                .stroke = {
                    .color = 0x00FF00FF,
                    .line_width = 0.5f 
                },
                .data.circle = {
                    // (junction  (at       X/Y  ) )
                    .center = {
                        val.children[0].children[0].number,                     
                        val.children[0].children[1].number,
                    },
                    .radius = 0.25f
                }
            };
            list_push(circuit.junctions, junction_shape);
            continue;
        }
        if(strcmp(val.identifier, "symbol") == 0){
            // debug_print_parse_value_recursive(val);
           
            //STOP OMITTING TOKENS PLEASE
            
            symbol_t symbol = (symbol_t){
                .unit = 1,
                .style = 1,
                .properties = list_init(shape_t),
            };
            char *lib_id;

            for(int i = 0; i < list_size(val.children); i++){
                parse_value_t sym_val = val.children[i];
                
                if(strcmp(sym_val.identifier, "lib_id") == 0){ 
                    lib_id = sym_val.children[0].string;

                    for(int i = 0; i < list_size(circuit.symbol_library); i++){ 
                        if(strcmp(circuit.symbol_library[i].name, lib_id) == 0){
                            symbol.lib_index = i;
                            break;
                        }
                    }

                    continue;
                }
                if(strcmp(sym_val.identifier, "at") == 0){ 
                    symbol.position = parse_position(sym_val);
                    symbol.rotation = (int)sym_val.children[2].number;
                    continue;
                }
                if(strcmp(sym_val.identifier, "unit") == 0){ 
                    symbol.unit = (int)sym_val.children[0].number;
                    continue;
                }
                if(strcmp(sym_val.identifier, "body_style") == 0){ 
                    symbol.style = (int)sym_val.children[0].number;
                    continue;
                }

                if(strcmp(sym_val.identifier, "property") == 0){

                    if(strlen(sym_val.children[1].string) == 0)
                        continue;
                    
                    bool hide = false;
                    point_t position;

                    for(int i = 0; i < list_size(sym_val.children); i++){
                        parse_value_t prop_value = sym_val.children[i];
                        if(strcmp(prop_value.identifier, "hide") == 0){
                            hide = sym_val.children[5].children[0].boolean;
                            break;
                        }
                        if(strcmp(prop_value.identifier, "at") == 0){
                            position = parse_position(sym_val.children[2]);
                        }
                    }
                    SDL_Log("adding property %s", sym_val.children[1].string);
                    
                    if(hide)
                        continue;
                    
                    char *text = str_dup(sym_val.children[1].string);
    
                    shape_t text_shape = {
                        .type = DRAW_TEXT,
                        .data.text = {
                            .string = text,
                            .position = position
                        }
                    };

                    list_push(symbol.properties, text_shape);
                    continue;
                }



            }
            
            SDL_Log("adding instance of %s (index %zu, unit %i, style %i)", lib_id, symbol.lib_index, symbol.unit, symbol.style);
            
            list_push(circuit.symbols, symbol);
            continue;
        }

    }

    free_expression(parsed_output);

    return circuit;
}

void add_symbol_type_to_library(parse_value_t parse_value, circuit_t *circuit){
    //debug_print_parse_value(parse_value);
    
    if(parse_value.type != VALUE_GROUP)
        return;
    SDL_assert(strcmp(parse_value.identifier, "symbol") == 0);

    symbol_type_t new_symbol = {
        .units = list_init(unit_type_t)
    };

    for(int i = 0; i < list_size(parse_value.children); i++){
        parse_value_t val = parse_value.children[i];
        //debug_print_parse_value(val);
        switch(val.type){
            case VALUE_STRING:
                new_symbol.name = str_dup(val.string);
                SDL_Log("adding component type %s", new_symbol.name);
                break;

            case VALUE_GROUP:
                //SDL_Log("%s", val.children[j].identifier);
                
                if(strcmp(val.identifier, "symbol"))
                    continue;

                add_unit_to_symbol_type(val, &new_symbol);

                break;
            default: break;
        }
    }

    list_push(circuit->symbol_library, new_symbol);
}

void add_unit_to_symbol_type(parse_value_t parse_value, symbol_type_t *type){
    char * unit_name = parse_value.children[0].string;
    SDL_Log("\tadding unit|style %s", unit_name);
  
    int unit_number, style_number;
  
    size_t unit_name_len = strlen(unit_name);

    if(unit_name_len < 3){
        SDL_Log("What the");
        return;
    }
    char *first_underline = unit_name + unit_name_len - 3;

    for(; first_underline >= unit_name && *first_underline != '_'; first_underline--);

    sscanf(first_underline, "_%i_%i", &unit_number, &style_number);
    
    SDL_Log("\t\tadding %zu shapes to %s", list_size(parse_value.children), parse_value.children[0].string);

    unit_type_t unit = {
        .unit = unit_number,
        .style = style_number,
        .graphics = list_init(shape_t)
    }; 


    for(int i = 1; i < list_size(parse_value.children); i++){
        add_shape_to_list(parse_value.children[i], &unit.graphics);
    }

    if(unit_number == 0 || style_number == 0){
        type->has_common_units = true;
    }
    
    list_push(type->units, unit);

}


void add_shape_to_list(parse_value_t parse_value, shape_t **list){
    if(parse_value.type != VALUE_GROUP){
        return;
    }
    
    if(strcmp(parse_value.identifier, "circle") == 0){
        shape_t circle = {
            .type = DRAW_CIRCLE,
            .data.circle = {
                .center = parse_position(parse_value.children[0]),
                .radius = parse_value.children[1].children[0].number
            },
            .stroke = {
                .color = 0x00FF00FF,
                .line_width = 0.5f,
                .line_style = STYLE_SOLID,
                .fill = false
            }
        };

        list_push(*list, circle);
    }else if(strcmp(parse_value.identifier, "rectangle") == 0){
        shape_t rect = {
            .type = DRAW_RECTANGLE,
            .data.rect = {
                .start = flip_y(parse_position(parse_value.children[0])),
                .end = flip_y(parse_position(parse_value.children[1]))
            },
            .stroke = {
                .color = 0x00FF00FF,
                .line_width = 0.5f,
                .line_style = STYLE_SOLID, 
                .fill = false
            }
        };
        
        list_push(*list, rect);
        
    }else if(strcmp(parse_value.identifier, "polyline") == 0){ 

        parse_value_t pts = parse_value.children[0];
        SDL_assert(pts.type == VALUE_GROUP && strcmp(pts.identifier, "pts") == 0);

        for(int i = 1; i < list_size(pts.children); i++){
            shape_t line = {
                .type = DRAW_LINE,
                .stroke = {
                    .color = 0x00FF00FF,
                    .line_width = 0.25f 
                },
                .data.line = {
                    .start = flip_y(parse_position(pts.children[i-1])),
                    .end = flip_y(parse_position(pts.children[i]))
                }
            };
            list_push(*list, line);
        }

    }else if(strcmp(parse_value.identifier, "arc") == 0){
        

        point_t start  = flip_y(parse_position(parse_value.children[0]));
        point_t middle = flip_y(parse_position(parse_value.children[1]));
        point_t end    = flip_y(parse_position(parse_value.children[2]));


        SDL_Log("start: %f %f", start.x, start.y);
        SDL_Log("middle: %f %f", middle.x, middle.y);
        SDL_Log("end: %f %f", end.x, end.y);

        point_t center = find_arc_center(start, middle, end);
        float radius = glm_vec2_distance(&center.x, &middle.x);

        SDL_Log("center: %f %f", center.x, center.y);

        point_t start_centered = point_sub(start, center);
        point_t end_centered = point_sub(end, center);

        shape_t arc = {
            .type = DRAW_ARC,
            .stroke = {
                .color = 0x00FF00FF,
                .line_width = 0.25f 
            },
            .data.arc = {
                .center = center, 
                .radius = radius,
                .start = start_centered,
                .end = end_centered,
            }
        };

        list_push(*list, arc);
    }else if(strcmp(parse_value.identifier, "pin") == 0){

        SDL_assert(parse_value.type == VALUE_GROUP && parse_value.children[2].type == VALUE_GROUP);
        point_t at = flip_y(parse_position(parse_value.children[2]));
        int rotation = (int)parse_value.children[2].children[2].number;
        float length = parse_value.children[3].children[0].number;
       
        point_t end = at;

        switch(rotation){
            case 0:
                end = (point_t){at.x + length, at.y};
                break;
            case 90:
                end = (point_t){at.x, at.y - length};
                break;
            case 180:
                end = (point_t){at.x - length, at.y};
                break;
            case 270:
                end = (point_t){at.x, at.y + length};
                break;
        }

        shape_t line = {
            .type = DRAW_LINE,
            .stroke = {
                .color = 0x00FF00FF,
                .line_width = 0.25f 
            },
            .data.line = {
                .start = at,
                .end = end,
            }
        };
        list_push(*list, line);

    }

}

shape_t *drawable_test(char *path){
    char *file = load_file(path);
    if(file == NULL)
        return NULL;
    
    parse_context_t ctx = {file};
    parse_value_t val = parse_expression(&ctx);

    shape_t *commands = list_init(shape_t);

    //add_draw_command(&commands, val);

    return commands;
}
