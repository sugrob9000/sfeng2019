#include "render.h"
#include "ent/lights.h"
#include "input/cmds.h"
#include "input/console.h"
#include "resource.h"
#include "vis.h"
#include <cassert>
#include <chrono>

t_sdlcontext sdlcont;

t_visible_set visible_set;

void draw_sky ();
void render_all ()
{
	namespace cr = std::chrono;
	using sc = cr::steady_clock;
	sc::time_point frame_start = sc::now();
	static float last_frame_time = 0.0;

	material_barrier();

	camera.apply();
	visible_set.fill(camera.pos);

	compute_lighting();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, sdlcont.res_x, sdlcont.res_y);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	draw_sky();
	visible_set.render(SHADE_FINAL);
	visible_set.render_debug();

	// HUD
	reset_matrices();
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glUseProgram(0);

	if (console_active)
		console_render();

	last_frame_time = cr::duration<float>(sc::now() - frame_start).count();
	SDL_GL_SwapWindow(sdlcont.window);
}


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

	int img_flags = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF;
	int img_success;
	if ((img_success = IMG_Init(img_flags)) != img_flags) {
		fatal("IMG init failed! attempted %x, successful %x",
			img_flags, img_success);
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
	                    SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	sdlcont.window = SDL_CreateWindow("Engine",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			sdlcont.res_x, sdlcont.res_y,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
				| SDL_WINDOW_RESIZABLE);
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
	glClearColor(1.0, 1.0, 1.0, 1.0);

	glEnable(GL_MULTISAMPLE);

	glMatrixMode(MTX_VIEWPROJ);
	glLoadIdentity();
	glMatrixMode(MTX_MODEL);
	glLoadIdentity();

	extern void init_text ();
	extern void init_sky ();
	extern void init_cuboid ();

	init_cuboid();

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

COMMAND_ROUTINE (windowsize)
{
	if (ev != PRESS)
		return;
	if (args.size() != 2)
		return;
	int w = std::atoi(args[0].c_str());
	int h = std::atoi(args[1].c_str());
	if (w <= 0 || h <= 0)
		return;
	resize_window(w, h);
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

void reset_matrices ()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void push_reset_matrices ()
{
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
}

void pop_matrices ()
{
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
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

	glMatrixMode(MTX_VIEWPROJ);
	glLoadIdentity();
	gluPerspective(fov, aspect, z_near, z_far);
	glRotatef(-90.0, 1.0, 0.0, 0.0);
	rotate_gl_matrix(ang);
	translate_gl_matrix(-pos);

	glMatrixMode(MTX_MODEL);
	glLoadIdentity();
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

	text_program = glCreateProgram();
	glAttachShader(text_program, get_vert_shader("common/text"));
	glAttachShader(text_program, get_frag_shader("common/text"));
	glLinkProgram(text_program);

	text_prg_glyph_loc = glGetUniformLocation(text_program, "glyphs");

	char all_chars[257];
	all_chars[0] = '~';
	all_chars[256] = 0;
	for (int i = 1; i < 256; i++)
		all_chars[i] = i;

	SDL_Surface* surf = TTF_RenderText_Blended(sdlcont.font,
			all_chars, text_color);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
	glEnable(GL_BLEND);

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
		glVertex2f(x, y - charh);
		glTexCoord2i(w * (c + 1), h);
		glVertex2f(x + charw, y - charh);
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
	glAttachShader(sky_shader_program, get_vert_shader("common/sky"));
	glAttachShader(sky_shader_program, get_frag_shader("common/sky"));
	glLinkProgram(sky_shader_program);
}

void draw_sky ()
{
	glUseProgram(sky_shader_program);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);

	constexpr float radius = 1.0;
	const t_bound_box skybox = { { -radius, -radius, -radius },
	                             { radius, radius, radius } };
	glPushMatrix();
	translate_gl_matrix(camera.pos);

	draw_cuboid(skybox);

	glPopMatrix();
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);
}

GLuint cuboid_dlist;
void init_cuboid ()
{
	vec3 p[8];
	for (int i = 0; i < 8; i++) {
		p[i] = { i & 1 ? 1.0f : -1.0f,
		         i & 2 ? 1.0f : -1.0f,
		         i & 4 ? 1.0f : -1.0f };
	}
	auto quad = [&p] (int a, int b, int c, int d)
	-> void {
		glVertex3f(p[a].x, p[a].y, p[a].z);
		glVertex3f(p[b].x, p[b].y, p[b].z);
		glVertex3f(p[c].x, p[c].y, p[c].z);
		glVertex3f(p[d].x, p[d].y, p[d].z);
	};

	cuboid_dlist = glGenLists(1);
	glNewList(cuboid_dlist, GL_COMPILE);
	glBegin(GL_QUADS);

	quad(0, 2, 3, 1);
	quad(0, 1, 5, 4);
	quad(0, 4, 6, 2);
	quad(7, 5, 1, 3);
	quad(7, 3, 2, 6);
	quad(7, 6, 4, 5);

	glEnd();
	glEndList();
}

void draw_cuboid (const t_bound_box& b)
{
	vec3 scale = (b.end - b.start) * 0.5;
	vec3 center = (b.end + b.start) * 0.5;

	glPushMatrix();
	translate_gl_matrix(center);
	glScalef(scale.x, scale.y, scale.z);
	glCallList(cuboid_dlist);
	glPopMatrix();
}

void debug_texture_onscreen (GLuint texture, float x, float y, float scale)
{
	glUseProgram(0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glEnable(GL_TEXTURE_2D);

	glPushMatrix();
	glTranslatef(x, y, 0.0);
	glScalef(scale, scale, 1.0);

	glColor4f(1.0, 1.0, 1.0, 1.0);
	glBegin(GL_QUADS);

	const int vx[4] = { 0, 1, 1, 0 };
	const int vy[4] = { 0, 0, 1, 1 };

	for (int i = 0; i < 4; i++) {
		glTexCoord2i(vx[i], vy[i]);
		glVertex2i(vx[i], vy[i]);
	}

	glEnd();
	glPopMatrix();
}


constexpr short cam_move_f = 0;
constexpr short cam_move_b = 1;
constexpr short cam_move_l = 2;
constexpr short cam_move_r = 3;
bool cam_move_flags[4];
t_camera camera;

bool cam_speedup = false;
bool cam_slowdown = false;
COMMAND_SET_BOOL (cam_accelerate, cam_speedup);
COMMAND_SET_BOOL (cam_decelerate, cam_slowdown);

void upd_camera_pos ()
{
	float speed = 4.0;
	if (cam_speedup)
		speed *= 2.5;
	if (cam_slowdown)
		speed *= 0.4;

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

MOUSEMOVE_ROUTINE (camera)
{
	camera.ang.x += dy;
	camera.ang.z += dx;
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

COMMAND_ROUTINE (cam_dump_pos)
{
	if (ev != PRESS)
		return;
	std::cout << "pos " << camera.pos
		<< "\nang " << camera.ang << std::endl;
}
