#include<SDL3/SDL.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include<cimgui/cimgui.h>
#include<string.h>
#include"simulation.h"
#include"parser/parser.h"
#include"list.h"
#include<math.h>

bool centralized_button(const char *label, float width){
    ImGuiStyle* style = igGetStyle();

    ImVec2 text_size;
    if(width <= 0){
        igCalcTextSize(&text_size, label, label + strlen(label), true, 200.0);  
    }else{
        text_size = (ImVec2){width, 0};
    }

    float size = text_size.x + (style->FramePadding.x * 2.0f);

    ImVec2 avail;
    igGetContentRegionAvail(&avail);

    float off = (avail.x - text_size.x) * 0.5;
    if (off > 0.0f)
        igSetCursorPosX(igGetCursorPosX() + off);

    return igButton(label, (ImVec2){width, 0});
}

bool show_style_editor = false;
bool show_simulation_controls = true;
bool show_demo_window = false;

float speed = 3.0f;
float step = 1.0f;

extern SDL_Window* window;
extern circuit_t current_circuit;


void open_file_callback(void* userdata, const char* const* filelist, int filter){
    SDL_Log("Importing file: '%s'", *filelist);
    
    if(filelist[0] == NULL)
        return;

    current_circuit = parse_schematic((char*)filelist[0]);

    // while (*filelist) {
    //     filelist++;
    // }
}

const SDL_DialogFileFilter filters[] = {
    { "KiCad Schematic",  "kicad_sch" },
    { "All files",   "*" }
};

bool sim_running_placeholder = false;
float timer = 0;

float noise(void *dat, int i){
    float t = i * 3.14159f * 0.05f + timer * 10.0f;
    return sinf(t) + (sinf(3.0f * t )/3.0f) + (sinf(5.0f * t)/5.0f) + (sinf(7.0f * t)/7.0f);
}

void simulation_controls(){
    igBegin("Simulation Controls", &show_simulation_controls, ImGuiWindowFlags_None);

    igText("Simulation Speed");
    igSliderFloat("##Sim_Speed", &speed, 0.1, 100.0, "%f steps/s", ImGuiSliderFlags_Logarithmic);

    igNewLine();

    if(!sim_running_placeholder){
        igPushStyleColor_U32(ImGuiCol_Button,        0xFF00BB00);
        igPushStyleColor_U32(ImGuiCol_ButtonHovered, 0xFF00DD00);
        igPushStyleColor_U32(ImGuiCol_ButtonActive,  0xFF00AA00);
        if(igButton("Start", (ImVec2){80.0, 30.0})){
            sim_running_placeholder = true;

            for(int i = 0; i < list_size(current_circuit.wires); i++){
                current_circuit.wires[i].stroke.line_width = 0.4f;
            }

        }
        igPopStyleColor(3);
    }else{

        igPushStyleColor_U32(ImGuiCol_Button,        0xFF0000BB);
        igPushStyleColor_U32(ImGuiCol_ButtonHovered, 0xFF0000DD);
        igPushStyleColor_U32(ImGuiCol_ButtonActive,  0xFF0000AA);
        if(igButton("Stop", (ImVec2){80.0, 30.0})){
            sim_running_placeholder = false; 
            for(int i = 0; i < list_size(current_circuit.wires); i++){
                current_circuit.wires[i].stroke.line_width = 0.25f;
                current_circuit.wires[i].stroke.color = 0x440000FF;
            }
        }
        igPopStyleColor(3);

        timer += 1.0f/165.0f * speed;

        for(int i = 0; i < list_size(current_circuit.wires); i++){    
            if(sin(timer * 3.14f + (i ^ 0x5457)) > 0.5f || cos(timer * -2.0f + (i ^ 0x7554)) > 0.5f){
                current_circuit.wires[i].stroke.color = 0xFF0000FF;
            }else{
                current_circuit.wires[i].stroke.color = 0x440000FF;
            }
        }

        igPlotLines_FnFloatPtr("Clock##2", noise, NULL, 200, 0, NULL, -1.0f, 1.0f, (ImVec2){0, 80});
    }

    igText("Time Elapsed: %fµs", timer);




    igEnd();
}


void gui(){
    
    ImGuiIO *io = igGetIO_Nil();

#ifndef IMGUI_HAS_VIEWPORT
    ImGuiViewport* viewport = igGetMainViewport();
    igSetNextWindowPos(viewport->WorkPos, ImGuiCond_Always, (ImVec2){0.0f, 0.0f});
    igSetNextWindowSize(viewport->WorkSize, ImGuiCond_Always);
    igSetNextWindowViewport(viewport->ID);
#else 
    igSetNextWindowPos((ImVec2){0.0f, 0.0f}, ImGuiCond_Always, (ImVec2){0.0f, 0.0f});
    igSetNextWindowSize(io->DisplaySize, ImGuiCond_Always);
#endif



    if(igBeginMainMenuBar()){
        if(igBeginMenu("File \uf413", true)){
            // if(igMenuItem_Bool("New", "Ctrl+N", false, true)){ 
            // } 
            if(igMenuItem_Bool("Open", "Ctrl+O", false, true)){
                SDL_ShowOpenFileDialog(open_file_callback, NULL, window, filters, SDL_arraysize(filters), NULL, false);
            } 

            // if(igMenuItem_Bool("Import", "Ctrl+I", false, true)){
            // }
            igEndMenu();
        }
        if(igBeginMenu("Edit \uf448", true)){
            if(igMenuItem_BoolPtr("GUI Style", "", &show_style_editor, true)){

            }
            igEndMenu();
        } 
        if(igBeginMenu("View \uf187", true)){

            if(igMenuItem_BoolPtr("Simulation", "", &show_simulation_controls, true)){ }
            if(igMenuItem_BoolPtr("ImGUI Demo", "", &show_demo_window, true)){ }
            igEndMenu();
        } 
        if(igBeginMenu("Help \uf128", true)){
            if(igMenuItem_Bool("Source", "", false, true)){
                SDL_OpenURL("https://github.com/hadron13/sPCB");
            } 
            igEndMenu();
        } 


        igEndMainMenuBar();
    }
    if(show_style_editor){
        igBegin("GUI Style Editor", &show_style_editor, ImGuiWindowFlags_None);
        igShowStyleEditor(igGetStyle());
        igEnd();
    }
    if (show_demo_window)
        igShowDemoWindow(&show_demo_window);
    if(show_simulation_controls)
        simulation_controls();
}










