#include "inc_general.h"
#include "inc_gl.h"
#include "render/render.h"
#include "render/resource.h"
#include "render/material.h"
#include "core/core.h"
#include "input/input.h"
#include "input/console.h"
#include "input/cmds.h"

namespace render
{

t_sdlcontext cont;

bool cam_move_flags[4];
t_camera camera;

void upd_camera_pos ()
{
	const float speed = 1.0;
	vec3 delta;
	auto& flags = cam_move_flags;
	t_camera& cam = camera;

	if (cam.ang.x < -90.0)
		cam.ang.x = -90.0;
	if (cam.ang.x > 90.0)
		cam.ang.x = 90.0;

	float sz = sinf(cam.ang.z * DEG_TO_RAD);
	float sx = sinf(cam.ang.x * DEG_TO_RAD);
	float cz = cosf(cam.ang.z * DEG_TO_RAD);

	if (flags[cam_move_f])
		delta += vec3(sz, cz, -sx);
	if (flags[cam_move_b])
		delta -= vec3(sz, cz, -sx);
	if (flags[cam_move_l])
		delta -= vec3(cz, -sz, 0.0);
	if (flags[cam_move_r])
		delta += vec3(cz, -sz, 0.0);

	delta.norm();
	cam.pos += delta * speed;
}

void render_all ()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	camera.apply();

	// TODO: visible sets
	for (const core::e_base* e: core::ents.vec)
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

void gl_msg_callback (GLenum source, GLenum type, GLenum id, GLenum severity,
		int msg_len, const char* msg, const void* param)
{
	core::warning("OpenGL: %s", msg);
}

void init_text ();
void init ()
{
	if (cont.res_x == 0 || cont.res_y == 0) {
		// resolution has not been initialized,
		// use a sane default
		cont.res_x = 640;
		cont.res_y = 480;
	}

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		core::fatal("SDL_Init failed: %s", SDL_GetError());

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
	                    SDL_GL_CONTEXT_PROFILE_CORE);

	cont.window = SDL_CreateWindow("Engine",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			cont.res_x, cont.res_y,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
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

	init_text();
}

void resize_window (int w, int h)
{
	cont.res_x = w;
	cont.res_y = h;
	SDL_SetWindowSize(cont.window, w, h);
	glViewport(0, 0, w, h);
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
	float aspect = (float) cont.res_x / cont.res_y;

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
unsigned int text_prg_glyph_loc;

void init_text ()
{
	if (TTF_Init() < 0)
		core::fatal("TTF init failed");
	cont.font = TTF_OpenFont(cont.font_path, cont.font_h);
	if (cont.font == nullptr)
		core::fatal("Failed to find font %s", cont.font_path);
	if (!TTF_FontFaceIsFixedWidth(cont.font)) {
		core::warning("Font %s is not monospace. Text will break",
				cont.font_path);
	}
	TTF_GlyphMetrics(cont.font, '~', nullptr, nullptr,
			nullptr, nullptr, &cont.font_w);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	t_shader_id vert = get_shader("vert_identity", GL_VERTEX_SHADER);
	t_shader_id frag = get_shader("frag_text", GL_FRAGMENT_SHADER);

	text_program = glCreateProgram();
	glAttachShader(text_program, vert);
	glAttachShader(text_program, frag);
	glLinkProgram(text_program);

	text_prg_glyph_loc = glGetUniformLocation(text_program, "glyphs");

	char all_chars[257];
	all_chars[0] = '~';
	all_chars[256] = 0;
	for (int i = 1; i < 256; i++)
		all_chars[i] = i;

	SDL_Surface* surf = TTF_RenderText_Blended(cont.font,
			all_chars, text_color);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGenTextures(1, &text_texture);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, text_texture);
	glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, GL_RGBA, surf->w, surf->h,
			0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels);

	SDL_FreeSurface(surf);
}

void draw_text (const char* str, int x, int y)
{
	glUseProgram(text_program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, text_texture);
	glUniform1i(text_prg_glyph_loc, 0);

	glBegin(GL_QUADS);
	for (int i = 0; str[i] != 0; i++) {
		const char c = str[i];

		glTexCoord2i(cont.font_w * c, 0);
		glVertex2i(x, y);
		glTexCoord2i(cont.font_w * c, cont.font_h);
		glVertex2i(x, y + cont.font_h);
		glTexCoord2i(cont.font_w * (c + 1), cont.font_h);
		glVertex2i(x + cont.font_w, y + cont.font_h);
		glTexCoord2i(cont.font_w * (c + 1), 0);
		glVertex2i(x + cont.font_w, y);

		x += cont.font_w;
	}
	glEnd();
}

} // namespace render

COMMAND_ROUTINE (move_cam)
{
	if (args.empty())
		return;

	auto& flags = render::cam_move_flags;
	bool f = (ev == PRESS);

	switch (tolower(args[0][0])) {
	case 'f':
		flags[render::cam_move_f] = f;
		break;
	case 'b':
		flags[render::cam_move_b] = f;
		break;
	case 'l':
		flags[render::cam_move_l] = f;
		break;
	case 'r':
		flags[render::cam_move_r] = f;
		break;
	}
}

MOUSEMOVE_ROUTINE (mousemove_camera)
{
	render::camera.ang.x += dy;
	render::camera.ang.z += dx;
}

COMMAND_ROUTINE (windowsize)
{
	if (ev != PRESS)
		return;
	if (args.size() != 2)
		return;
	int w = std::atoi(args[0].c_str());
	int h = std::atoi(args[1].c_str());
	if (w == 0 || h == 0)
		return;

	render::resize_window(w, h);
}
