#include "src/render/render.h"
#include<glad/gl.h>
#include<stdint.h>
#include<stdlib.h>
#include<stdio.h>
#include<math.h>

#include<SDL3/SDL.h>

#include"../data.h"
#include"../list.h"
#include"../../cglm/cglm.h"



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
        printf("Shader compilation failed [%s]: %s\n", path, shader_log);
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

// 0xRRGGBBAA -> {R, G, B, A}
void color_to_vec4(uint32_t color, float *vec){
    vec[0] = (float)((color >> 24) & 0xFF)/255.0f;
    vec[1] = (float)((color >> 16) & 0xFF)/255.0f;
    vec[2] = (float)((color >> 8 ) & 0xFF)/255.0f;
    vec[3] = (float)((color      ) & 0xFF)/255.0f;
}


static unsigned int VAO = 0;
static unsigned int VBO = 0;
static unsigned int shader;

static GLuint rect_shader;
static GLuint circle_shader;
static GLuint line_shader;

static int zoom;
static point_t offset;
static point_t window_size;
static mat4 projection;

draw_command_t *drawable_test(char *path);
draw_command_t *commands;

void render_init(){
    commands = drawable_test("test22.kicad_sym");
    SDL_Log("commands: %i", list_size(commands));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float vertices[] = {
    //  position         UV
        0.0f, 1.0f,      0.0f, 1.0f,
        0.0f, 0.0f,      0.0f, 0.0f,
        1.0f, 0.0f,      1.0f, 0.0f,

        0.0f, 1.0f,      0.0f, 1.0f,
        1.0f, 0.0f,      1.0f, 0.0f,
        1.0f, 1.0f,      1.0f, 1.0f
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

    window_size = (point_t){1200, 800};
    glm_ortho(-600, 600, 400, -400, 0.1, 10.0f, projection);
    
    rect_shader = shader_compile("data/shaders/standard.vert.glsl", "data/shaders/rect.frag.glsl");
    circle_shader = shader_compile("data/shaders/standard.vert.glsl", "data/shaders/circle.frag.glsl");
    line_shader = shader_compile("data/shaders/standard.vert.glsl", "data/shaders/line.frag.glsl");
}

void update_projection(){
    double scale = pow(1.1, (double)-zoom);
    glm_ortho((float)-window_size.x/2.0f * scale - offset.x, 
              (float) window_size.x/2.0f * scale - offset.x, 
              (float) window_size.y/2.0f * scale - offset.y, 
              (float)-window_size.y/2.0f * scale - offset.y, 0.1, 10.0f, projection);
}

void render_mouse_scroll(int ticks){
    zoom += ticks;
    update_projection();
}
void render_mouse_drag(float x, float y){
    double scale = pow(1.1, (double)-zoom);
    offset.x += x * scale;
    offset.y += y * scale;
    update_projection();
}

void render_update_resolution(int x, int y){
    window_size = (point_t){x, y};
    update_projection();
}


void render_draw_shape(draw_command_t command){   

    int shader_id;
    point_t quad_origin, quad_size;

    float t = (double)SDL_GetTicks()/1000.0f;
    float rotation = 0;
    float margin = command.stroke.line_width; 

    vec4 color_vec;
    color_to_vec4(command.stroke.color, color_vec);

    switch(command.type){
        case DRAW_RECTANGLE:
            if(command.data.rect.end.x < command.data.rect.start.x){
                float temp = command.data.rect.start.x;
                command.data.rect.start.x = command.data.rect.end.x;
                command.data.rect.end.x = temp;
            }
            if(command.data.rect.end.y < command.data.rect.start.y){
                float temp = command.data.rect.start.y;
                command.data.rect.start.y = command.data.rect.end.y;
                command.data.rect.end.y = temp;
            }

            quad_origin = command.data.rect.start;
            quad_size.x = command.data.rect.end.x - command.data.rect.start.x;
            quad_size.y = command.data.rect.end.y - command.data.rect.start.y;
            shader_id = rect_shader;
            break;
        case DRAW_CIRCLE:
            quad_origin = command.data.circle.center;
            quad_origin.x -= command.data.circle.radius;
            quad_origin.y -= command.data.circle.radius;
            
            float diam = command.data.circle.radius * 2.0f;
            quad_size = (point_t){diam, diam};

            shader_id = circle_shader;
            break;
        case DRAW_LINE:
            
            quad_origin = command.data.line.start; 
            float xrel = command.data.line.end.x - command.data.line.start.x;
            float yrel = command.data.line.end.y - command.data.line.start.y;
            float dist = hypot(xrel, yrel);

            quad_size = (point_t){0, dist};

            rotation = atan2(xrel, yrel);

            shader_id = line_shader;
            break;
        case DRAW_ARC:

            break;  
        default:
            break;
    }

    mat4 transform;
    glm_translate_make(transform, (vec3){quad_origin.x - margin/2.0f, quad_origin.y - margin/2.0f, 0});
    
    glm_translate(transform, (vec3){(margin)/2.0f,  ( margin)/2.0f, 0});
    glm_rotate(transform, rotation, (vec3){0, 0, -1.0});
    glm_translate(transform, (vec3){-(margin)/2.0f, -(margin)/2.0f, 0});

    glm_scale(transform, (vec3){quad_size.x + margin, quad_size.y + margin, 1.0});
    

    glUseProgram(shader_id);
    int tloc = glGetUniformLocation(shader_id, "transform");
    int ploc = glGetUniformLocation(shader_id, "projection");
    int cloc = glGetUniformLocation(shader_id, "color");
    int origin_loc = glGetUniformLocation(shader_id, "quad_origin");
    int size_loc   = glGetUniformLocation(shader_id, "quad_size");
    int thickness_loc = glGetUniformLocation(shader_id, "thickness");

    glUniformMatrix4fv(ploc, 1, false, (float*)projection);
    glUniformMatrix4fv(tloc, 1, false, (float*)transform);
    glUniform4fv(cloc, 1, (float*)color_vec);
    glUniform2fv(origin_loc, 1, &quad_origin.x);
    glUniform2fv(size_loc, 1, &quad_size.x);
    glUniform1f(thickness_loc, command.stroke.line_width);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}



void render_draw(){ 

    float t = (double)SDL_GetTicks()/5000.0f;

    draw_command_t test = {
        .type = DRAW_RECTANGLE,
        .stroke = {
            .color = 0xFF0000FF,
            .line_width = 30.0f 
        },
        .data.rect = {
            .start = {-100, -100},
            .end = {100, 100}
        }
    };
    draw_command_t test2 = {
        .type = DRAW_CIRCLE,
        .stroke = {
            .color = 0xFF0000FF,
            .line_width = 40.0f 
        },
        .data.circle = {
            .center = {500, 500},
            .radius = 200
        }
    };
    draw_command_t test3 = {
        .type = DRAW_LINE,
        .stroke = {
            .color = 0x00FF00FF,
            .line_width = 5.0f 
        },
        .data.line = {
            .start = {300, 300},
            .end = {600, 300}
        }
    };
    // if(rand()%5== 1){
    //     SDL_Log("x %f y %f", sin(t), cos(t));
    // }

    for(int i = 0; i < list_size(commands); i++){
        render_draw_shape(commands[i]);
    }
    //render_draw_shape(test2);
    render_draw_shape(test3);
}
