#include<glad/gl.h>
#include<stdint.h>
#include<stdlib.h>
#include<stdio.h>

#include"../data.h"
#include"../../cglm/cglm.h"
#include "cglm/mat4.h"

const float quad_verts[] = {
    0.0, 0.0,
    1.0, 0.0,
    0.0, 1.0,
    0.0, 1.0,
    1.0, 0.0,
    1.0, 1.0
};


char *load_file(const char *path){
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

int compile_shader_stage(int type, const char *path) {
    const char *source = load_file(path);
    if(source == NULL){
        return 0;
    }

    int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char shader_log[1024];
        glGetShaderInfoLog(shader, 1024, NULL, shader_log);
        printf("Shader compilation failed: %s\n", shader_log);
    }
    return shader;
}

int shader_compile(const char *vertex_path, const char *fragment_path){ 
    int vertex = compile_shader_stage(GL_VERTEX_SHADER, vertex_path);
    int fragment = compile_shader_stage(GL_FRAGMENT_SHADER, fragment_path);

    // link shader program
    int program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);

    // cleanup
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    return program;
}




static unsigned int VAO = 0;
static unsigned int VBO = 0;
static unsigned int shader;

void render_init(){
    float vertices[] = {
    //  position         UV
        0.0f, 1.0f,      0.0f, 0.0f,
        0.0f, 0.0f,      0.0f, 1.0f,
        1.0f, 0.0f,      1.0f, 1.0f,

        0.0f, 1.0f,      0.0f, 0.0f,
        1.0f, 0.0f,      1.0f, 1.0f,
        1.0f, 1.0f,      1.0f, 0.0f
    };

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    shader = shader_compile("data/shaders/standard.vert.glsl", "data/shaders/circle.frag.glsl");
}

void render_draw_shape(draw_command_t command){   
    

    point_t quad_origin, quad_size;

    switch(command.type){
        case DRAW_RECTANGLE:
            quad_origin = command.data.rect.start;
            quad_size.x = command.data.rect.end.x - command.data.rect.start.x;
            quad_size.y = command.data.rect.end.y - command.data.rect.start.y;
            break;
        default:
            break;
    }

    mat4 transform, projection;
    glm_mat4_identity(transform);
    glm_translate(transform, (vec3){quad_origin.x, quad_origin.y, 0});
    glm_scale(transform, (vec3){quad_size.x, quad_size.y, 1.0});

    glUseProgram(shader);
    glm_ortho(0, 1200, 800, 0, 0.1, 10.0f, projection);



    int tloc = glGetUniformLocation(shader, "transform");
    int ploc = glGetUniformLocation(shader, "projection");

    glUniformMatrix4fv(ploc, 1, false, (float*)projection);
    glUniformMatrix4fv(tloc, 1, false, (float*)transform);



    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}


void render_draw(){ 

    draw_command_t test = {
        .type = DRAW_RECTANGLE,
        .color = 0xFF0000,
        .data.rect = {
            .start = {100, 100},
            .end = {400, 400}
        }
    };

    render_draw_shape(test);
}
