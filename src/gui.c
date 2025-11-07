#include "src/circuit_data.h"
#include<SDL3/SDL.h>
#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include<cimgui/cimgui.h>
#include<string.h>
#include"simulation.h"

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


    igBegin("main", NULL, ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoSavedSettings);

    circuit_t *circuit = simulation_circuit();
   
    for(int j = 0; j < 2; j++){
        component_t *component = &circuit->components[j];
        for(int i = 0; i < 7; i++){
            igSetCursorPosX(200);
            igSetCursorPosY(i * 10 + j * 100);
            igText("%.2f", component->pins[i].voltage);
            igSetCursorPosY(i * 10 + j * 100);
            igSetCursorPosX(240);
            igText("%i", i);
        }
        for(int i = 7; i < 14; i++){
            igSetCursorPosY((13 - i) * 10 + j * 100);
            igSetCursorPosX(280);
            igText("%i", i);
            igSetCursorPosX(300);
            igSetCursorPosY((13 - i) * 10 + j * 100);
            igText("%.2f", component->pins[i].voltage);
        }
    }


    igEnd();

    if(igBeginMainMenuBar()){
        if(igBeginMenu("File", true)){
            if(igMenuItem_Bool("New", "Ctrl+N", false, true)){
                
            } 
            if(igMenuItem_Bool("Open", "Ctrl+O", false, true)){

            } 
            if(igMenuItem_Bool("Import", "Ctrl+I", false, true)){

            }   
            igEndMenu();
        }
        if(igBeginMenu("Edit", true)){
            igEndMenu();
        } 
        if(igBeginMenu("Help", true)){
            if(igMenuItem_Bool("Source", "", false, true)){
                SDL_OpenURL("https://github.com/hadron13/sPCB");
            } 
            igEndMenu();
        } 


        igEndMainMenuBar();
    }




}












