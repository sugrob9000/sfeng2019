#ifndef RENDER_H
#define RENDER_H

#include "inc_gl.h"
#include "inc_general.h"
#include "core/core.h"

enum t_render_stage: uint32_t
{
	LIGHTING_LSPACE = 0,
	LIGHTING_SSPACE = 1,
	SHADE_FINAL = 2,

	NUM_RENDER_STAGES
};
constexpr int UNIFORM_LOC_RENDER_STAGE = 0;

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
extern t_sdlcontext sdlcont;

extern t_camera camera;
void upd_camera_pos ();

struct t_visible_set;
extern t_visible_set visible_set;

void rotate_gl_matrix (vec3 angs);
void translate_gl_matrix (vec3 pos);

void reset_matrices ();
void push_reset_matrices ();
void pop_matrices ();

void init_render ();
void render_all ();
void resize_window (int w, int h);

const SDL_Color text_color = { 200, 200, 200, 255 };
void draw_text (const char* text, float x, float y, float charw, float charh);

void draw_cuboid (const t_bound_box& b);

void debug_texture_onscreen (GLuint texture);


/*
 * A framebuffer object for general (but limited) purposes.
 * Does not support much customisation, but one can make()
 * one without any bits and set custom target textures
 */
struct t_fbo
{
	GLuint id;
	GLuint tex_color;
	GLuint tex_depth;

	int width;
	int height;

	static constexpr uint8_t BIT_COLOR = 0x1;
	static constexpr uint8_t BIT_ALPHA = 0x2;
	static constexpr uint8_t BIT_DEPTH = 0x4;
	void make (int w, int h, uint8_t bits);
	void make (int w, int h) { make(w, h, 0); }

	void attach_color (GLuint texture);
	void attach_depth (GLuint texture);

	void apply () const;
};

#endif // RENDER_H

