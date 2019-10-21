#ifndef RENDER_H
#define RENDER_H

#include "inc_gl.h"
#include "inc_general.h"

namespace render
{

struct t_camera
{
	vec3 pos;
	vec3 ang;

	float z_far;
	float z_near;
	float fov;

	/*
	 * Cannot specify the camera without
	 * the essentials such as fov and z-planes
	 */
	t_camera (vec3, vec3) = delete;
	t_camera (vec3) = delete;

	t_camera ();
	t_camera (vec3 pos, vec3 ang,
	          float zf, float zn, float fov);

	/*
	 * Transform the current projection matrix
	 * to correspond to the camera
	 */
	void apply ();
};

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
extern t_sdlcontext cont;

extern t_camera camera;

/*
 * How the camera is currently moving (demo)
 */
constexpr short cam_move_f = 0;
constexpr short cam_move_b = 1;
constexpr short cam_move_l = 2;
constexpr short cam_move_r = 3;
extern bool cam_move_flags[4];

void init ();
void render_all ();
void resize_window (int w, int h);

const SDL_Color text_color = { 200, 200, 200, 255 };
void draw_text (const char* text, int x, int y);

}

#endif // RENDER_H

