
#include<SDL3/SDL.h>
#include <SDL3/SDL_oldnames.h>
#include<glad/gl.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include<cimgui/cimgui.h>

#define CIMGUI_USE_SDL3
#define CIMGUI_USE_OPENGL3
#include<cimgui/cimgui_impl.h>

#include"simulation.h"

void gui();

int main(){
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

    SDL_Window * window = SDL_CreateWindow("sPCB", 1200, 800, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    
    if(window == NULL){
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Could not create a window");
    }

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);

    if(gl_context == NULL){
        SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "Could not create an OpenGL context");
    }

    
    if(!SDL_GL_SetSwapInterval(-1)){
        SDL_GL_SetSwapInterval(1);
    }
    
    if(gladLoadGL(SDL_GL_GetProcAddress) == 0){
        SDL_LogError(SDL_LOG_CATEGORY_GPU, "Could not load OpenGL");
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
    ImGuiStyle_ScaleAllSizes(style, main_scale);     
    style->FontScaleDpi = main_scale;        
    io->ConfigDpiScaleFonts = true;          
    io->ConfigDpiScaleViewports = true;      

    ImGui_ImplSDL3_InitForOpenGL(window, gl_context);
    ImGui_ImplOpenGL3_Init("#version 150");


    igStyleColorsDark(NULL);

    simulation_init();

    bool running = true;
    bool show_demo_window = true;
    while(running){

        SDL_Event event;
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_EVENT_QUIT){
                running = false;
            }
            if(event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED && event.window.windowID == SDL_GetWindowID(window)){
                running = false;
            }
            ImGui_ImplSDL3_ProcessEvent(&event);
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        igNewFrame();
        static int timer = 0;
        timer++;
        if(timer == 165){
            simulation_step();
            timer = 0;
        }
        gui();

        if (show_demo_window)
            igShowDemoWindow(&show_demo_window);

        igRender();
        SDL_GL_MakeCurrent(window, gl_context);
        glViewport(0, 0, (int)io->DisplaySize.x, (int)io->DisplaySize.y);
        glClearColor(0.0, 0.0, 0.0, 1.0); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

