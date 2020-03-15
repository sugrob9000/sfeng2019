#ifndef RENDER_H
#define RENDER_H

#include "core/core.h"
#include "inc_gl.h"
#include "misc.h"
#include "render/ctx.h"


struct t_sdlcontext
{
	SDL_Window* window;
	SDL_GLContext glcont;
	SDL_Renderer* renderer;
	int res_x;
	int res_y;

	const char* font_path = "res/FreeMono.ttf";
	TTF_Font* font;
	static constexpr int font_h = 16;
	int font_w;
};
extern t_sdlcontext sdlcont;


struct t_visible_set;
extern t_visible_set visible_set;

void init_render ();
void render_all ();
void resize_window (int w, int h);

const SDL_Color text_color = { 200, 200, 200, 255 };
void draw_text (const char* text, float x, float y, float charw, float charh);

void debug_texture_onscreen (GLuint texture, float x, float y, float scale);


extern GLuint cuboid_dlist_inwards;
extern GLuint cuboid_dlist_outwards;

#endif // RENDER_H

