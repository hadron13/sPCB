#pragma once


#include "src/data.h"
void render_init();
void render_update_resolution(int w, int h);
void render_mouse_scroll(int ticks);
void render_mouse_drag(float x, float y);
void render_draw();
void render_draw_circuit(circuit_t*);


