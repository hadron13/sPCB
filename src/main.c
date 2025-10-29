
#include<SDL3/SDL.h>
#include<glad/gl.h>

#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include<cimgui/cimgui.h>

#define CIMGUI_USE_SDL3
#define CIMGUI_USE_OPENGL3
#include<cimgui/cimgui_impl.h>


void simulation_init();
void simulation_step();

int main(){
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    
    float main_scale = SDL_GetDisplayContentScale(SDL_GetPrimaryDisplay());

    SDL_Window * window = SDL_CreateWindow("title", 800, 600, SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
    
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
            ImGui_ImplSDL3_ProcessEvent(&event);
            if(event.type == SDL_EVENT_QUIT){
                running = false;
            }
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        igNewFrame();

        glClearColor(0.0, 0.0, 0.0, 1.0); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        simulation_step();

        if (show_demo_window)
            igShowDemoWindow(&show_demo_window);
        // SDL_Delay(700);
        ImGui_ImplOpenGL3_RenderDrawData(igGetDrawData());
        
        SDL_GL_SwapWindow(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();

    SDL_GL_DestroyContext(gl_context);

    SDL_DestroyWindow(window); 
    SDL_Quit();
}

