
#include "src/data.h"
#include<SDL3/SDL.h>
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_oldnames.h>
#include <SDL3/SDL_video.h>
#include<glad/gl.h>
#include <wchar.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include<cimgui/cimgui.h>

#define CIMGUI_USE_SDL3
#define CIMGUI_USE_OPENGL3
#include<cimgui/cimgui_impl.h>

#include"simulation.h"
#include"render/render.h"
#include"parser/parser.h"

SDL_Window * window;
circuit_t  current_circuit;

void gui();

int main(){
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

    window = SDL_CreateWindow("sPCB", 1200, 800, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    
    if(window == NULL){
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Could not create a window");
        return -1;
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);

    if(gl_context == NULL){
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Could not create an OpenGL context");
        return -1;
    }

    
    if(!SDL_GL_SetSwapInterval(-1)){
        SDL_GL_SetSwapInterval(1);
    }
    
    if(gladLoadGL(SDL_GL_GetProcAddress) == 0){
        SDL_LogError(SDL_LOG_CATEGORY_GPU, "Could not load OpenGL");
        return -1;
    }

    SDL_Log("loaded OpenGL version %s", glGetString(GL_VERSION));
    SDL_Log("vendor: %s", glGetString(GL_VENDOR)); 
 
    igCreateContext(NULL);
    ImGuiIO* io = igGetIO_Nil(); (void)io;
    io->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; 

#ifdef IMGUI_HAS_DOCK
    io->ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
    io->ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
#endif    

    // Setup Dear ImGui style
    igStyleColorsDark(NULL);
    //igStyleColorsLight(NULL);
    // Setup scaling
    ImGuiStyle* style = igGetStyle();

    style->ItemSpacing.x = 10;
    style->WindowRounding = 6.0f;
    


    ImGuiStyle_ScaleAllSizes(style, main_scale);     
    style->FontScaleDpi = main_scale;        
    io->ConfigDpiScaleFonts = true;          
    io->ConfigDpiScaleViewports = true;      

    ImFontConfig *font_config = ImFontConfig_ImFontConfig();

    ImFontAtlas_AddFontFromFileTTF(io->Fonts, "data/fonts/JetBrainsMonoNLNerdFont-Regular.ttf", 0, font_config, NULL);

    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 150");


    igStyleColorsDark(NULL);

    // circuit_t circuit = parse_schematic("D:\\sPCB\\led\\led.kicad_sch");
    //circuit_t circuit = parse_schematic("D:\\sPCB\\test3.kicad_sch");
    // current_circuit = parse_schematic("/home/pico/development/M6502/clock_gen.kicad_sch");
    current_circuit= parse_schematic("/home/pico/development/spcb/led/led.kicad_sch");

    simulation_init();
    render_init();


    bool running = true;
    bool show_demo_window = true;

    bool mouse_left_pressed = false, mouse_right_pressed = false, mouse_middle_pressed = false;

    while(running){

        SDL_Event event;
        while(SDL_PollEvent(&event)){
            switch(event.type){
                case SDL_EVENT_QUIT:
                    running = false;
                    break;
                case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
                    if(event.window.windowID == SDL_GetWindowID(window))
                        running = false;
                    break;
                case SDL_EVENT_MOUSE_BUTTON_DOWN:
                    switch(event.button.button){
                        case 1: mouse_left_pressed   = true; break;
                        case 2: mouse_middle_pressed = true; break;
                        case 3: mouse_right_pressed  = true; break;
                    }
                    break;
                case SDL_EVENT_MOUSE_BUTTON_UP:
                    switch(event.button.button){
                        case 1: mouse_left_pressed   = false; break;
                        case 2: mouse_middle_pressed = false; break;
                        case 3: mouse_right_pressed  = false; break;
                    }
                    break;
                case SDL_EVENT_MOUSE_WHEEL:
                    render_mouse_scroll(event.wheel.integer_y);
                    break;
                case SDL_EVENT_MOUSE_MOTION:
                    if(mouse_middle_pressed || mouse_right_pressed)
                        render_mouse_drag(event.motion.xrel, event.motion.yrel);
                    break;
                case SDL_EVENT_WINDOW_RESIZED:
                    if(event.window.windowID == SDL_GetWindowID(window)){
                        render_update_resolution(event.window.data1, event.window.data2);
                    }
                    break;
                case SDL_EVENT_WINDOW_MOVED:
                    if(event.window.windowID == SDL_GetWindowID(window)){
                        render_update_position(event.window.data1, event.window.data2);
                    }
                    break;
                case SDL_EVENT_KEY_DOWN:
                    switch(event.key.key){
                        case SDLK_R:
                            current_circuit = parse_schematic(current_circuit.path);
                            break;
                    }
                    break;

            }
            ImGui_ImplSDL3_ProcessEvent(&event);
        }


        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        igNewFrame();
        static int timer = 0;
        timer++;
        if(timer == 165){
            //simulation_step();
            timer = 0;
        }


        gui();
       


        ImDrawList* fg = igGetForegroundDrawList_ViewportPtr(NULL);
        ImU32 col  = 0xFFFFFFFF;


        ImVec2 window_pos, window_size;
        int x, y;
        SDL_GetWindowPosition(window, &x, &y);
        // igGetWindowPos(&window_pos);
        // SDL_Log("%i %i", x, y);
        ImVec2 pos = {x + 30.0f, y + 50.0f};

        ImDrawList_AddText_FontPtr(fg, igGetDefaultFont(), 24, pos, col, current_circuit.filename, NULL, 0, NULL);



        if (show_demo_window)
           igShowDemoWindow(&show_demo_window);

        SDL_GL_MakeCurrent(window, gl_context);
        glViewport(0, 0, (int)io->DisplaySize.x, (int)io->DisplaySize.y);
        glClearColor(0.0, 0.0, 0.1, 1.0); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        {
            render_draw_background();
            render_draw_circuit(&current_circuit);
            igRender();
        }



        ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
#ifdef IMGUI_HAS_DOCK
	if (io->ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            SDL_Window* backup_current_window = SDL_GL_GetCurrentWindow();
            SDL_GLContext backup_current_context = SDL_GL_GetCurrentContext();
            igUpdatePlatformWindows();
            igRenderPlatformWindowsDefault(NULL,NULL);
            SDL_GL_MakeCurrent(backup_current_window, backup_current_context);
        }
#endif


        
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    igDestroyContext(NULL);

    SDL_GL_DestroyContext(gl_context);

    SDL_DestroyWindow(window); 
    SDL_Quit();
}

