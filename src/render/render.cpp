#include "inc_general.h"
#include "inc_gl.h"
#include "render/render.h"
#include "render/resource.h"
#include "render/material.h"
#include "core/core.h"
#include "input/input.h"
#include "input/console.h"

namespace render
{

t_sdlcontext cont;

t_camera camera;

void render_all ()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	camera.apply();

	// TODO: visible sets
	for (const core::e_base* e: core::game.ents.v)
		e->render();

	// HUD
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glUseProgram(0);

	if (input::console.active)
		input::console.render();

	SDL_GL_SwapWindow(cont.window);
}

bool cam_move_flags[4];

void init_text_drawing ();
void init (int resx, int resy)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		core::fatal("SDL_Init failed: %s", SDL_GetError());

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
	                    SDL_GL_CONTEXT_PROFILE_CORE);

	cont.window = SDL_CreateWindow("Engine",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			resx, resy, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	cont.res_x = resx;
	cont.res_y = resy;

	if (cont.window == nullptr)
		core::fatal("SDL window creation failed: %s", SDL_GetError());

	cont.glcont = SDL_GL_CreateContext(cont.window);

	if (cont.glcont == nullptr)
		core::fatal("SDL glcont creation failed: %s", SDL_GetError());

	glewExperimental = true;
	if (glewInit() != GLEW_OK)
		core::fatal("GLEW init failed");

	cont.renderer = SDL_CreateRenderer(cont.window, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (TTF_Init() < 0)
		core::fatal("TTF init failed");
	cont.font = TTF_OpenFont("res/FreeMono.ttf", cont.font_size);
	if (cont.font == nullptr)
		core::fatal("Failed to find font");
	init_text_drawing();
}

t_camera::t_camera () { }

t_camera::t_camera (
		vec3 apos, vec3 aang,
		float zf, float zn, float afov)
{
	pos = apos;
	ang = aang;
	z_far = zf;
	z_near = zn;
	fov = afov;
}

void t_camera::apply ()
{
	constexpr float aspect = 4.0 / 3.0;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, aspect, z_near, z_far);
	glRotatef(-90.0, 1.0, 0.0, 0.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glRotatef(ang.x, 1.0, 0.0, 0.0);
	glRotatef(ang.y, 0.0, 1.0, 0.0);
	glRotatef(ang.z, 0.0, 0.0, 1.0);
	glTranslatef(-pos.x, -pos.y, -pos.z);
}


t_texture_id text_texture;
unsigned int text_program;
unsigned int text_program_loc;
int single_char_width;

void init_text_drawing ()
{
	t_shader_id vert = get_shader("vert_identity", GL_VERTEX_SHADER);
	t_shader_id frag = get_shader("frag_text", GL_FRAGMENT_SHADER);

	text_program = glCreateProgram();
	glAttachShader(text_program, vert);
	glAttachShader(text_program, frag);
	glLinkProgram(text_program);

	text_program_loc = glGetUniformLocation(text_program, "tex");

	char* all_chars = new char[256];
	for (int i = 1; i < 256; i++)
		all_chars[i] = i;
	all_chars[0] = '#';

	SDL_Surface* surf = TTF_RenderText_Solid(cont.font,
			all_chars, text_color);

	delete[] all_chars;

	glGenTextures(1, &text_texture);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, text_texture);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGB, surf->w, surf->h,
			0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels);

	SDL_FreeSurface(surf);

	// query the width for a pretty wide character to be safe
	TTF_GlyphMetrics(cont.font, 'm', nullptr, nullptr,
			nullptr, &single_char_width, nullptr);
}

void draw_text (const char* str, int x, int y)
{
	glUseProgram(text_program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, text_texture);
	glUniform1i(text_program_loc, 0);

	glBegin(GL_QUADS);
	for (int i = 0; str[i] != 0; i++) {
		const char c = str[i];

		glTexCoord2f(single_char_width * c, 0.0);
		glVertex2i(x, y);
		glTexCoord2f(single_char_width * c, cont.font_size);
		glVertex2i(x, y + cont.font_size);
		glTexCoord2f(single_char_width * (c + 1), cont.font_size);
		glVertex2i(x + single_char_width, y + cont.font_size);
		glTexCoord2f(single_char_width * (c + 1), 0.0);
		glVertex2i(x + single_char_width, y);

		x += single_char_width;
	}
	glEnd();
}

}
