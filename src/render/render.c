#include "src/render/render.h"
#include <SDL3/SDL_timer.h>
#include <SDL3/SDL_video.h>
#include<glad/gl.h>
#include<stdint.h>
#include<stdlib.h>
#include<stdio.h>
#include<math.h>

#include<SDL3/SDL.h>
#include <wchar.h>

#include"../data.h"
#include"../list.h"
#include"../../cglm/cglm.h"
#include "cglm/mat4.h"

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include "cimgui/cimgui.h"



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
    
    SDL_Log("compiled shader program [%s | %s]", vertex_path, fragment_path);

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
static GLuint arc_shader;
static GLuint background_shader;

static int zoom;
static point_t offset;
static point_t window_pos;
static point_t window_size;
static mat4 projection;

// shape_t *drawable_test(char *path);
// shape_t *commands;

void update_projection();

void render_init(){
    // commands = drawable_test("test22.kicad_sym");
    // SDL_Log("commands: %i", list_size(commands));

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
    
    zoom = 20.0;
    offset.x -= 100;
    offset.y -= 100;
    update_projection();

    rect_shader = shader_compile("data/shaders/standard.vert.glsl", "data/shaders/rect.frag.glsl");
    circle_shader = shader_compile("data/shaders/standard.vert.glsl", "data/shaders/circle.frag.glsl");
    line_shader = shader_compile("data/shaders/standard.vert.glsl", "data/shaders/line.frag.glsl");
    arc_shader = shader_compile("data/shaders/standard.vert.glsl", "data/shaders/arc.frag.glsl");
    background_shader = shader_compile("data/shaders/background.vert.glsl", "data/shaders/background.frag.glsl");
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
void render_update_position(int x, int y){
    window_pos = (point_t){x, y};
}

void render_draw_background(){ 
    glUseProgram(background_shader);

    double scale = pow(1.1, (double)-zoom);

    int size_loc = glGetUniformLocation(background_shader, "quad_size");
    int origin_loc = glGetUniformLocation(background_shader, "quad_origin");
    
    glUniform2f(size_loc, window_size.x * scale, -window_size.y * scale);
    glUniform2f(origin_loc, offset.x, offset.y);
    
    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
}

ImDrawList *imgui_drawlist;

void render_draw_shape(shape_t command, point_t offset, float rotation){   

    int shader_id;
    point_t quad_origin, quad_size;
    float t = (double)SDL_GetTicks()/1000.0f;
    float quad_rotation = 0;
    float margin = command.stroke.line_width; 

    vec4 color_vec;
    color_to_vec4(command.stroke.color, color_vec);

    switch(command.type){

        case DRAW_TEXT:
            if(imgui_drawlist == NULL)
                return;

            float current_font_size = igGetFontSize();
            ImVec2 text_size;
            igCalcTextSize(&text_size, command.data.text.string, NULL, false, 0);


            vec4 ndc;
            glm_mat4_mulv(projection, (vec4){command.data.text.position.x, command.data.text.position.y, -1.0, 1.0}, ndc);

            ImVec2 pos = {window_pos.x + ((ndc[0] + 1.0) / 2.0) * window_size.x, window_pos.y + ((-ndc[1] + 1.0) / 2.0) * window_size.y};

            pos.y -= ((text_size.y/2.0f)/current_font_size)*zoom;
            if(command.data.text.justify == JUSTIFY_CENTER){
                pos.x -= ((text_size.x/2.0f)/current_font_size)*zoom;
            }
             
            ImDrawList_AddText_FontPtr(imgui_drawlist, igGetDefaultFont(), glm_max(zoom, 0.5f), pos, 0xFFFFFFFF, command.data.text.string, NULL, 0, NULL);

            return;

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
            quad_rotation = atan2(xrel, yrel);

            shader_id = line_shader;
            break;
        case DRAW_ARC:
            quad_origin = command.data.arc.center;
            quad_origin.x -= command.data.arc.radius;
            quad_origin.y -= command.data.arc.radius;
            
            float arc_diam = command.data.arc.radius * 2.0f;
            quad_size = (point_t){arc_diam, arc_diam};
            
            shader_id = arc_shader;

            glUseProgram(shader_id);
            int start_loc = glGetUniformLocation(shader_id, "start");
            int end_loc = glGetUniformLocation(shader_id, "end");

            glUniform2fv(start_loc, 1, &command.data.arc.start.x);
            glUniform2fv(end_loc, 1, &command.data.arc.end.x);

            break;  
        default:
            break;
    }

    mat4 transform;
    glm_mat4_identity(transform);


    glm_translate(transform, (vec3){offset.x, offset.y, 0});
    glm_rotate(transform, rotation, (vec3){0, 0, -1.0});

    glm_translate(transform, (vec3){quad_origin.x - margin/2.0f, quad_origin.y - margin/2.0f, 0}); 

    glm_translate(transform, (vec3){(margin)/2.0f,  ( margin)/2.0f, 0});
    glm_rotate(transform, quad_rotation, (vec3){0, 0, -1.0});
    glm_translate(transform, (vec3){-(margin)/2.0f, -(margin)/2.0f, 0});
    
    glm_scale(transform, (vec3){quad_size.x + margin, quad_size.y + margin, 1.0});
    

    glUseProgram(shader_id);
    int transform_loc = glGetUniformLocation(shader_id, "transform");
    int proj_loc = glGetUniformLocation(shader_id, "projection");
    int color_loc = glGetUniformLocation(shader_id, "color");
    int origin_loc = glGetUniformLocation(shader_id, "quad_origin");
    int size_loc   = glGetUniformLocation(shader_id, "quad_size");
    int thickness_loc = glGetUniformLocation(shader_id, "thickness");

    glUniformMatrix4fv(proj_loc, 1, false, (float*)projection);
    glUniformMatrix4fv(transform_loc, 1, false, (float*)transform);
    glUniform4fv(color_loc, 1, (float*)color_vec);
    glUniform2fv(origin_loc, 1, &quad_origin.x);
    glUniform2fv(size_loc, 1, &quad_size.x);
    glUniform1f(thickness_loc, command.stroke.line_width);

    glBindVertexArray(VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}



void render_draw(){ 

    float t = (double)SDL_GetTicks()/5000.0f;


}




void render_draw_symbol(symbol_type_t *library, symbol_t *symbol){
    symbol_type_t type = library[symbol->lib_index]; 
    
    if((symbol->cached_unit == NULL && list_size(type.units) >= 1) || (type.has_common_units && symbol->cached_common_unit == NULL)){
        // SDL_Log("caching %s of unit %i style %i", type.name, symbol->unit, symbol->style);

        for(int i = 0; i < list_size(type.units); i++){
            unit_type_t *unit = &type.units[i];
            
            if(unit->unit == 0){
                symbol->cached_common_unit = unit;
                // SDL_Log("common: %i_%i", unit->unit, unit->style);
                continue;
            }
            if(unit->unit == symbol->unit){
                if(unit->style == 0){
                    symbol->cached_common_unit = unit;
                    // SDL_Log("common(style): %i_%i", unit->unit, unit->style);
                    continue;
                }else if(unit->style == symbol->style){ 
                    symbol->cached_unit = unit;
                    // SDL_Log("proper: %i_%i", unit->unit, unit->style);
                    continue;
                }
            }
        }   
        // SDL_Log("%zu", SDL_GetPerformanceCounter());
    }


    // for(int u = 0; u < list_size(type.units); u++){
    //     for(int i = 0; i < list_size(type.units[u].graphics); i++){
    //         render_draw_shape(type.units[u].graphics[i], symbol->position, glm_rad(symbol->rotation));
    //     }
    // } 
    //
    if(type.has_common_units && symbol->cached_common_unit != NULL){ 
        for(int i = 0; i < list_size(symbol->cached_common_unit->graphics); i++){
            render_draw_shape(symbol->cached_common_unit->graphics[i], symbol->position, glm_rad(symbol->rotation));
        }
    }

    if(symbol->cached_unit != NULL){
        for(int i = 0; i < list_size(symbol->cached_unit->graphics); i++){
            render_draw_shape(symbol->cached_unit->graphics[i], symbol->position, glm_rad(symbol->rotation));
        }
    }

    if(!imgui_drawlist)
        return;

    for(int i = 0; i < list_size(symbol->properties); i++){ 
        render_draw_shape(symbol->properties[i], (point_t){0, 0}, 0);
    }

}

void render_draw_circuit(circuit_t *circuit){

    if(circuit == NULL)
        return;


    for(int i = 0; i < list_size(circuit->wires); i++){
        render_draw_shape(circuit->wires[i], (point_t){0, 0}, 0);
    }
    for(int i = 0; i < list_size(circuit->junctions); i++){
        render_draw_shape(circuit->junctions[i], (point_t){0, 0}, 0);
    }

    // for(int i = 0; i < list_size(circuit->symbol_library); i++){
    //     for(int j = 0; j < list_size(circuit->symbol_library[i].units); j++){
    //         for(int k = 0; k < list_size(circuit->symbol_library[i].units[j].graphics); k++){
    //             render_draw_shape(circuit->symbol_library[i].units[j].graphics[k], (point_t){10.0f * i}, 0);
    //         }
    //     }
    // }

    imgui_drawlist = igGetBackgroundDrawList(NULL);

    for(int i = 0; i < list_size(circuit->symbols); i++){
        render_draw_symbol(circuit->symbol_library, &circuit->symbols[i]);
    }
    

}

