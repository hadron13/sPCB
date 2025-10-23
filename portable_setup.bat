@echo off
mkdir build


if not exist cimgui\Makefile (
    git submodule update --init --recursive
)

if not exist w64devkit\ (
    curl -L https://github.com/skeeto/w64devkit/releases/download/v2.4.0/w64devkit-x64-2.4.0.7z.exe --ssl-no-revoke > w64devkit.exe
    if exist w64devkit.exe (
        .\w64devkit.exe
        del w64devkit.exe
    )
)

if not exist sdl3\ (
    curl -L https://github.com/libsdl-org/SDL/releases/download/release-3.2.24/SDL3-devel-3.2.24-mingw.zip --ssl-no-revoke > libsdl3.zip
    powershell -command "Expand-Archive -Path 'libsdl3.zip' -DestinationPath '.\sdl3'"
    move sdl3\SDL3-3.2.24\x86_64-w64-mingw32\bin\SDL3.dll build
    del libsdl3.zip 
)
