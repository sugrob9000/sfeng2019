#include "core/core.h"
#include "core/entity.h"
#include "inc_general.h"
#include "inc_gl.h"
#include "input/cmds.h"
#include "input/console.h"
#include "input/input.h"
#include "render/material.h"
#include "render/render.h"
#include "render/resource.h"
#include "render/vis.h"
#include <chrono>

t_sdlcontext sdlcont;

constexpr short cam_move_f = 0;
constexpr short cam_move_b = 1;
constexpr short cam_move_l = 2;
constexpr short cam_move_r = 3;
bool cam_speedup = false;
bool cam_move_flags[4];
t_camera camera;

void upd_camera_pos ()
{
	float speed = cam_speedup ? 10.0 : 4.0;
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

COMMAND_ROUTINE (cam_move)
{
	if (args.empty())
		return;

	auto& flags = cam_move_flags;
	bool f = (ev == PRESS);

	switch (tolower(args[0][0])) {
	case 'f':
		flags[cam_move_f] = f;
		break;
	case 'b':
		flags[cam_move_b] = f;
		break;
	case 'l':
		flags[cam_move_l] = f;
		break;
	case 'r':
		flags[cam_move_r] = f;
		break;
	}
}

COMMAND_ROUTINE (cam_accelerate)
{
	cam_speedup = (ev == PRESS);
}

void draw_sky ();
void render_all ()
{
	namespace cr = std::chrono;
	using sc = cr::steady_clock;

	sc::time_point frame_start = sc::now();
	static float last_frame_sec;

	camera.apply();
	vis_fill_visible(camera.pos);

	compute_lighting();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, sdlcont.res_x, sdlcont.res_y);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	draw_sky();
	for (const oct_node* node: visible_leaves)
		node->render_tris();
	for (const e_base* e: ents.vec)
		e->render();

	vis_debug_renders();

	// HUD
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glScalef(1.0, -1.0, 1.0);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glUseProgram(0);

	char str[128];
	sprintf(str, "%i leaves", (int) visible_leaves.size());
	draw_text(str, -1, -1, 0.025, 0.05);
	sprintf(str, "%i fps", (int) round(1.0 / last_frame_sec));
	draw_text(str, -1, -1 + 0.06, 0.025, 0.05);

	debug_texture_onscreen(sspace_fbo_texture, 0.0, 0.0);

	if (console_active)
		console_render();

	last_frame_sec = cr::duration<float>(sc::now() - frame_start).count();

	SDL_GL_SwapWindow(sdlcont.window);
}


void init_text ();
void init_sky ();
void init_render ()
{
	if (sdlcont.res_x == 0 || sdlcont.res_y == 0) {
		// resolution has not been initialized,
		// use a sane default
		sdlcont.res_x = 640;
		sdlcont.res_y = 480;
	}

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		fatal("SDL_Init failed: %s", SDL_GetError());

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
	                    SDL_GL_CONTEXT_PROFILE_CORE);

	sdlcont.window = SDL_CreateWindow("Engine",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			sdlcont.res_x, sdlcont.res_y,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (sdlcont.window == nullptr)
		fatal("SDL window creation failed: %s", SDL_GetError());

	sdlcont.glcont = SDL_GL_CreateContext(sdlcont.window);
	if (sdlcont.glcont == nullptr)
		fatal("SDL glcont creation failed: %s", SDL_GetError());

	glewExperimental = true;
	if (glewInit() != GLEW_OK)
		fatal("GLEW init failed");

	sdlcont.renderer = SDL_CreateRenderer(sdlcont.window, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (SDL_GL_SetSwapInterval(-1) == -1) {
		warning("Failed to set adaptive vsync, setting regular");
		SDL_GL_SetSwapInterval(1);
	}

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	init_materials();
	init_text();
	init_vis();

	init_lighting();
	init_sky();
}

void resize_window (int w, int h)
{
	sdlcont.res_x = w;
	sdlcont.res_y = h;
	SDL_SetWindowSize(sdlcont.window, w, h);
	glViewport(0, 0, w, h);
}

void rotate_gl_matrix (vec3 angs)
{
	glRotatef(angs.x, 1.0, 0.0, 0.0);
	glRotatef(angs.y, 0.0, 1.0, 0.0);
	glRotatef(angs.z, 0.0, 0.0, 1.0);
}

void translate_gl_matrix (vec3 pos)
{
	glTranslatef(pos.x, pos.y, pos.z);
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
	float aspect = (float) sdlcont.res_x / sdlcont.res_y;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(fov, aspect, z_near, z_far);
	glRotatef(-90.0, 1.0, 0.0, 0.0);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	rotate_gl_matrix(ang);
	translate_gl_matrix(-pos);
}


t_texture_id text_texture;
unsigned int text_program;
unsigned int text_prg_glyph_loc;

void init_text ()
{
	if (TTF_Init() < 0)
		fatal("TTF init failed");
	sdlcont.font = TTF_OpenFont(sdlcont.font_path, sdlcont.font_h);
	if (sdlcont.font == nullptr)
		fatal("Failed to find font %s", sdlcont.font_path);
	if (!TTF_FontFaceIsFixedWidth(sdlcont.font)) {
		warning("Font %s is not monospace. Text will break",
				sdlcont.font_path);
	}
	TTF_GlyphMetrics(sdlcont.font, '~', nullptr, nullptr,
			nullptr, nullptr, &sdlcont.font_w);

	glEnable(GL_BLEND);

	text_program = glCreateProgram();
	glAttachShader(text_program,
			get_shader("common/vert_identity", GL_VERTEX_SHADER));
	glAttachShader(text_program,
			get_shader("common/frag_text", GL_FRAGMENT_SHADER));
	glLinkProgram(text_program);

	text_prg_glyph_loc = glGetUniformLocation(text_program, "glyphs");

	char all_chars[257];
	all_chars[0] = '~';
	all_chars[256] = 0;
	for (int i = 1; i < 256; i++)
		all_chars[i] = i;

	SDL_Surface* surf = TTF_RenderText_Blended(sdlcont.font,
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

void draw_text (const char* str, float x, float y, float charw, float charh)
{
	glUseProgram(text_program);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_RECTANGLE_ARB, text_texture);
	glUniform1i(text_prg_glyph_loc, 0);

	glBegin(GL_QUADS);
	int w = sdlcont.font_w;
	int h = sdlcont.font_h;

	for (int i = 0; str[i] != 0; i++) {
		const char c = str[i];
		glTexCoord2i(w * c, 0);
		glVertex2f(x, y);
		glTexCoord2i(w * c, h);
		glVertex2f(x, y + charh);
		glTexCoord2i(w * (c + 1), h);
		glVertex2f(x + charw, y + charh);
		glTexCoord2i(w * (c + 1), 0);
		glVertex2f(x + charw, y);
		x += charw;
	}
	glEnd();
}

GLuint sky_shader_program;
void init_sky ()
{
	sky_shader_program = glCreateProgram();
	t_shader_id vert = get_shader("common/vert_sky", GL_VERTEX_SHADER);
	t_shader_id frag = get_shader("common/frag_sky", GL_FRAGMENT_SHADER);
	glAttachShader(sky_shader_program, vert);
	glAttachShader(sky_shader_program, frag);
	glLinkProgram(sky_shader_program);
}

void draw_sky ()
{
	glUseProgram(sky_shader_program);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);

	constexpr float radius = 1.0;
	const t_bound_box skybox = { { -radius, -radius, -radius },
	                             { radius, radius, radius } };
	glPushMatrix();
	translate_gl_matrix(camera.pos);

	vis_render_bbox(skybox);

	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
}


void debug_texture_onscreen (GLuint texture, float cx, float cy)
{
	glUseProgram(0);
	glEnable(GL_TEXTURE_2D);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);

	glPushMatrix();
	glTranslatef(cx, cy, 0.0);

	glColor4f(1.0, 1.0, 1.0, 1.0);
	glBegin(GL_QUADS);
	glTexCoord2i(0, 0);
	glVertex2i(0, 1);
	glTexCoord2i(0, 1);
	glVertex2i(0, 0);
	glTexCoord2i(1, 1);
	glVertex2i(1, 0);
	glTexCoord2i(1, 0);
	glVertex2i(1, 1);
	glEnd();
	glPopMatrix();
}


MOUSEMOVE_ROUTINE (mousemove_camera)
{
	camera.ang.x += dy;
	camera.ang.z += dx;
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

	resize_window(w, h);
}
