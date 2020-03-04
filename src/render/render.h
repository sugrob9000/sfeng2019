#ifndef RENDER_H
#define RENDER_H

#include "inc_gl.h"
#include "misc.h"
#include "core/core.h"

enum t_render_stage: uint32_t
{
	LIGHTING_LSPACE = 0,
	LIGHTING_SSPACE = 1,
	SHADE_FINAL = 2,

	NUM_RENDER_STAGES
};

struct t_render_ctx
{
	t_render_stage stage;

	mat4 proj;
	mat4 view;
	mat4 model;
};
extern t_render_ctx render_ctx;


struct t_sdlcontext
{
	SDL_Window* window;
	SDL_GLContext glcont;
	SDL_Renderer* renderer;
	int res_x;
	int res_y;
	const char* const font_path = "res/FreeMono.ttf";
	TTF_Font* font;
	static constexpr int font_h = 16;
	int font_w;
};
extern t_sdlcontext sdlcont;

struct t_visible_set;
extern t_visible_set visible_set;

void rotate_gl_matrix (vec3 angs);
void translate_gl_matrix (vec3 pos);

void reset_matrices ();
void push_reset_matrices ();
void push_matrices ();
void pop_matrices ();

void init_render ();
void render_all ();
void resize_window (int w, int h);

const SDL_Color text_color = { 200, 200, 200, 255 };
void draw_text (const char* text, float x, float y, float charw, float charh);

void draw_cuboid (const t_bound_box& b);

void debug_texture_onscreen (GLuint texture, float x, float y, float scale);

#endif // RENDER_H

