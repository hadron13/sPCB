@echo off
setlocal
set PATH=.\w64devkit\bin;%PATH%
set PATH=..\w64devkit\bin;%PATH%

if not exist build\libs (    
    mkdir build\libs
)

set SDL3_DIR=sdl3/SDL3-3.2.24/x86_64-w64-mingw32

if not exist build\libs\libcimgui.a (
    cd cimgui
    make static
    cd ..
    move cimgui\libcimgui.a build\libs\
)
if not exist build\libs\cimgui_impl.o (
    g++ cimgui\imgui\backends\imgui_impl_sdl3.cpp    -DIMGUI_IMPL_API="extern \"C\" __declspec(dllexport)" -c -Icimgui/imgui -I%SDL3_DIR%/include  -o build\libs\cimgui_impl_sdl3.o 
    g++ cimgui\imgui\backends\imgui_impl_opengl3.cpp -DIMGUI_IMPL_API="extern \"C\" __declspec(dllexport)" -c -Icimgui/imgui -o build\libs\cimgui_impl_opengl3.o 
)
if not exist build\libs\glad.o (
    gcc glad/src/gl.c -c -o build/libs/glad.o -I glad/include
)

gcc src/*.c  build/libs/*.o -o build/spcb.exe -lSDL3 -lcimgui -mwindows -Iglad/include -I%SDL3_DIR%/include -I. -Lbuild -Lbuild/libs -lstdc++
