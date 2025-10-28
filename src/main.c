
#include<SDL3/SDL.h>
#include<glad/gl.h>


void simulation_init();
void simulation_step();

int main(){
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

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

    simulation_init();

    bool running = true;
    while(running){

        SDL_Event event;
        while(SDL_PollEvent(&event)){
            if(event.type == SDL_EVENT_QUIT){
                running = false;
            }
        }

        glClearColor(0.0, 0.0, 0.0, 1.0); 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        simulation_step();
        SDL_Delay(700);

        SDL_GL_SwapWindow(window);
    }

    SDL_GL_DestroyContext(gl_context);

    SDL_DestroyWindow(window); 
    SDL_Quit();
}

