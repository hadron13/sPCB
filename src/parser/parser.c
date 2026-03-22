#include"parser.h"
#include <SDL3/SDL_log.h>
#include <stdint.h>
#include <stdio.h>
#include <ctype.h>
#include "../list.h"

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



parse_value_t parse_expression(parse_context_t *ctx){
   
    parse_value_t value;

    while(isspace(*ctx->p)) ctx->p++; 

    if(*ctx->p == '('){
        ctx->p++;
       
        value.type = VALUE_GROUP;
        value.identifier = copy_token_advance(ctx);
        value.children = list_init(parse_value_t);

        while(*ctx->p != ')' && *ctx->p != '\0'){
            parse_value_t child_value = parse_expression(ctx);
            list_push(value.children, child_value);
        }
        ctx->p++;
        
    }else{ 
        char *token = copy_token_advance(ctx);

        if(token[0] == '"'){

            value.type = VALUE_STRING;
            value.string = str_dup1(token + 1);

        }else if(isdigit(token[0])){
            
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
        free(token);
    }


    return value;
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

void parse_schematic(char *path){
    
    char *file = load_file(path);
    if(file == NULL)
        return;
    
    
    parse_context_t ctx = {file};
    parse_value_t val = parse_expression(&ctx);
    

}



