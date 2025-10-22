@echo off
setlocal
set PATH=.\w64devkit\bin;%PATH%

mkdir build\libs

set SDL3_DIR=sdl3/SDL3-3.2.24/x86_64-w64-mingw32
set MYSQL_DIR=mysql\mysql-8.4.6-winx64

if not exist build\libs\libcimgui.a (
    cd cimgui
    make static
    cd ..
    move cimgui\libcimgui.a build\libs\
)
if not exist build\libs\cimgui_impl.o (
    g++ src\cimgui_sdl2_renderer\cimgui_impl.cpp -fPIC -c -Icimgui/imgui -o build\libs\cimgui_impl.o -Isdl2\SDL2-2.32.10\x86_64-w64-mingw32\include\SDL2
)

gcc src\*.c -o build\spcb_admin.exe -lmingw32 -lSDL3 -mwindows -lcimgui -lstdc++ build\libs\cimgui_impl.o -Lbuild -Lbuild\libs -L%SDL3_DIR%/lib -I%SDL3_DIR%/include -L%MYSQL_DIR%\lib\
