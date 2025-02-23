#pragma once
#include "inc_gl.h"

void init_render_debug ();

void debug_render_tex2d (GLuint id, float x_left, float y_bottom, float size);
void debug_render_tex2d_array (GLuint id, int layer,
		float x_left, float y_bottom, float size);