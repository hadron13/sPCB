mkdir -p build
mkdir -p build/libs

if [ ! -f build/libs/cimgui_impl.o ]; then
    clang++ src/cimgui_sdl2_renderer/cimgui_impl.cpp -fPIC -c -Icimgui/imgui -o build/libs/cimgui_impl.o $(sdl2-config --cflags)
fi
if [ ! -f build/libs/libcimgui.a ]; then
    make -C cimgui static
    mv cimgui/libcimgui.a build/libs
fi

# clang src/*.c -o build/spcb -lSDL3 -Lbuild/libs -lcimgui -lm -lGL -lstdc++ build/libs/cimgui_impl.o
clang src/*.c -o build/spcb -lSDL3 -lGL -lm -Iglad/include
