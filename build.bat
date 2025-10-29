@echo off
setlocal
set PATH=.\w64devkit\bin;%PATH%
set PATH=..\w64devkit\bin;%PATH%

mkdir build\libs

set SDL3_DIR=sdl3/SDL3-3.2.24/x86_64-w64-mingw32

if not exist build\libs\libcimgui.a (
    cd cimgui
    make static
    cd ..
    move cimgui\libcimgui.a build\libs\
)
if not exist build\libs\cimgui_impl.o (
    @REM g++ cimgui\imgui\backends\imgui_impl_opengl3.cpp cimgui\imgui\backends\imgui_impl_opengl3.cpp cimgui\imgui\backends\imgui_impl_sdl3.cpp -DCIMGUI_USE_SDL3 -DCIMGUI_USE_OPENGL3 -DIMGUI_IMPL_API="extern \"C\" __declspec(dllexport)" -fPIC -lSDL3 -Icimgui/imgui/backends -I%SDL3_DIR%/include -Icimgui/imgui -o build\libs\cimgui_impl.o 
)
if not exist build\libs\glad.o (
    gcc glad/src/gl.c -c -o build/libs/glad.o -I glad/include
)

gcc src/*.c build/libs/libcimgui.a build/libs/cimgui_impl.o build/libs/glad.o -o build/spcb.exe -lSDL3 -mwindows -Iglad/include -I%SDL3_DIR%/include -I. -Lbuild -lstdc++