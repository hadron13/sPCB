#include"parser.h"
#include <SDL3/SDL_log.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include <SDL3/SDL_assert.h>
#include "../list.h"

typedef enum{
    VALUE_EMPTY, VALUE_NUMBER, VALUE_BOOLEAN, VALUE_STRING, VALUE_GROUP
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
            if(strcmp(value.identifier, "lib_symbols") == 0){
                debug_print_val = true;
            }
            parse_value_t child_value = parse_expression(ctx);
            list_push(value.children, child_value);
            while(isspace(*ctx->p)) ctx->p++; 
        }
        ctx->p++; 
        
    }else{ 
        char *token = copy_token_advance(ctx);

        if(strlen(token) == 0){
            // ?????
            value.type = VALUE_EMPTY;
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

void debug_print_parse_value(parse_value_t value){
    if(value.type == VALUE_EMPTY){
        SDL_Log("<empty>");
        return;
    }

    SDL_Log("type: %s", value.type == VALUE_GROUP? "group": value.type == VALUE_NUMBER? "number": value.type == VALUE_STRING? "string" : "boolean");
    switch(value.type){
        case VALUE_GROUP: SDL_Log("id: %s - children: %zu", value.identifier, list_size(value.children)); break;
        case VALUE_BOOLEAN: SDL_Log("%s", value.boolean?"true":"false"); break;
        case VALUE_NUMBER: SDL_Log("%lf", value.number); break;
        case VALUE_STRING: SDL_Log("%p", value.string);break;
    }
}

void add_component_type_to_library(parse_value_t parse_value, circuit_t *circuit);
void add_symbol_to_component_type(parse_value_t parse_value, component_type_t *type);
void add_draw_command(draw_command_t **list, parse_value_t val);
void add_wire(parse_value_t parse_value, circuit_t *circuit);

void parse_schematic(char *path){
    
    char *file = load_file(path);
    if(file == NULL)
        return;

    SDL_Log("Parsing file %s", path);
    
    parse_context_t ctx = {file};
    parse_value_t parsed_output = parse_expression(&ctx);

    free(file);

    if(strcmp(parsed_output.identifier, "kicad_sch") != 0){
        SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Expected schematic as input!");
        return;
    }

    circuit_t circuit = {
        .symbol_library = list_init(component_type_t),
        .schematics = list_init(schematic_t),
        .components = list_init(component_t),
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
                debug_print_parse_value(val.children[j]);
                //SDL_Log("%s", val.children[j].identifier);
                

                if(val.children[j].type != VALUE_GROUP || strcmp(val.children[j].identifier, "symbol"))
                    continue;
                

                add_component_type_to_library(val.children[j], &circuit);
            }
            continue;
        }
    }

    free_expression(parsed_output);

}

void add_component_type_to_library(parse_value_t parse_value, circuit_t *circuit){
    //debug_print_parse_value(parse_value);
    
    if(parse_value.type != VALUE_GROUP)
        return;
    SDL_assert(strcmp(parse_value.identifier, "symbol") == 0);

    component_type_t new_component;

    for(int i = 0; i < list_size(parse_value.children); i++){
        parse_value_t val = parse_value.children[i];
        //debug_print_parse_value(val);
        switch(val.type){
            case VALUE_STRING:
                new_component.name = str_dup(val.string);
                SDL_Log("adding component type %s", new_component.name);
                break;

            case VALUE_GROUP:
                
                //SDL_Log("%s", val.children[j].identifier);
                
                if(strcmp(val.identifier, "symbol"))
                    continue;
                
                SDL_Log("adding subsymbol %s", val.children[0].string);
                
                break;
        }

    }
    list_push(circuit->symbol_library, new_component);
}

void add_symbol_to_component_type(parse_value_t parse_value, component_type_t *type){


}


void add_draw_command(draw_command_t **list, parse_value_t val){
    if(val.type != VALUE_GROUP){
        return;
    }
    
    if(strcmp(val.identifier, "circle") == 0){
        draw_command_t circle = {
            .type = DRAW_CIRCLE,
            .data.circle = {
                .center = (point_t){val.children[0].children[0].number, val.children[0].children[1].number},
                .radius = val.children[1].children[0].number
            },
            .stroke = {
                .color = 0x00FF00FF,
                .line_width = 0.5f,
                .line_style = STYLE_SOLID,
                .fill = false
            }
        };

        list_push(*list, circle);
    }else if(strcmp(val.identifier, "rectangle") == 0){
        draw_command_t rect = {
            .type = DRAW_RECTANGLE,
            .data.rect = {
                .start = (point_t){val.children[0].children[0].number, val.children[0].children[1].number},
                .end = (point_t){val.children[1].children[0].number, val.children[1].children[1].number}
            },
            .stroke = {
                .color = 0x00FF00FF,
                .line_width = 0.5f,
                .line_style = STYLE_SOLID, 
                .fill = false
            }
        };
        
        list_push(*list, rect);
        
    }else if(strcmp(val.identifier, "polyline") == 0){

    }else{
        for(int i = 0; i < list_size(val.children); i++){
            add_draw_command(list, val.children[i]);
        }
    }

}

draw_command_t *drawable_test(char *path){
    char *file = load_file(path);
    if(file == NULL)
        return NULL;
    
    parse_context_t ctx = {file};
    parse_value_t val = parse_expression(&ctx);

    draw_command_t *commands = list_init(draw_command_t);

    add_draw_command(&commands, val);

    return commands;
}
