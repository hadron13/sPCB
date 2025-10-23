@echo off
setlocal
set PATH=.\w64devkit\bin;%PATH%
set PATH=.\w64devkit\bin;%PATH%

mkdir build\libs

set SDL3_DIR=sdl3/SDL3-3.2.24/x86_64-w64-mingw32

@REM if not exist build\libs\libcimgui.a (
@REM     cd cimgui
@REM     make static
@REM     cd ..
@REM     move cimgui\libcimgui.a build\libs\
@REM )
@REM if not exist build\libs\cimgui_impl.o (
@REM     g++ src\cimgui_sdl2_renderer\cimgui_impl.cpp -fPIC -c -Icimgui/imgui -o build\libs\cimgui_impl.o -Isdl2\SDL2-2.32.10\x86_64-w64-mingw32\include\SDL2
@REM )

gcc src/*.c glad/src/gl.c -o build/spcb.exe -lSDL3 -mwindows -Iglad/include -I%SDL3_DIR%/include -Lbuild
