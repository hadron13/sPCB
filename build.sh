mkdir -p build
mkdir -p build/libs

if [ ! -f build/libs/cimgui_impl_opengl3.o ]; then
    clang++ cimgui/imgui/backends/imgui_impl_opengl3.cpp -c -o build/libs/cimgui_impl_opengl3.o -Icimgui/imgui -DIMGUI_IMPL_API="extern \"C\""
fi

if [ ! -f build/libs/cimgui_impl_sdl3.o ]; then
    clang++ cimgui/imgui/backends/imgui_impl_sdl3.cpp -c -o build/libs/cimgui_impl_sdl3.o -Icimgui/imgui -DIMGUI_IMPL_API="extern \"C\""
fi

if [ ! -f build/libs/libcimgui.a ]; then
    make -C cimgui static
    mv cimgui/libcimgui.a build/libs
fi

clang src/*.c build/libs/*.o glad/src/gl.c -o build/spcb -lSDL3 -lcimgui -lm -lGL -lstdc++ -Iglad/include -I. -Lbuild/libs
