#include "ent/lights.h"
#include "input/cmds.h"
#include "render/render.h"
#include "render/resource.h"
#include "render/sky.h"
#include "render/vis.h"
#include "render/framebuffer.h"
#include "render/gbuffer.h"
#include <cassert>
#include <chrono>

t_sdlcontext sdlctx;
t_render_ctx render_ctx;
t_visible_set visible_set;

void render_all ()
{
	namespace cr = std::chrono;
	using sc = cr::steady_clock;
	sc::time_point frame_start = sc::now();
	static float last_frame_time = -1.0;

	camera.apply();
	visible_set.fill(camera.pos);

	fill_gbuffers();
	compute_lighting();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, sdlctx.res_x, sdlctx.res_y);
	glClear(GL_DEPTH_BUFFER_BIT);

	render_ctx.stage = SHADE_FINAL;
	render_sky();
	visible_set.render();
	visible_set.render_debug();

	// HUD

	if (console_active)
		console_render();

	if (int err = glGetError(); err != 0)
		warning("OpenGL error 0x%x (%i)", err, err);

	last_frame_time = cr::duration<float>(sc::now() - frame_start).count();
	SDL_GL_SwapWindow(sdlctx.window);
}


void init_render ()
{
	if (sdlctx.res_x == 0 || sdlctx.res_y == 0) {
		// use a sane default
		sdlctx.res_x = 640;
		sdlctx.res_y = 480;
	}

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
		fatal("SDL_Init failed: %s", SDL_GetError());

	int img_flags = IMG_INIT_JPG | IMG_INIT_PNG | IMG_INIT_TIF;
	int img_success = IMG_Init(img_flags);
	if (img_success != img_flags) {
		fatal("IMG init failed: attempted %x, successful %x",
			img_flags, img_success);
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
	                    SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

	sdlctx.window = SDL_CreateWindow("churn-engine",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			sdlctx.res_x, sdlctx.res_y,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
				| SDL_WINDOW_RESIZABLE);
	if (sdlctx.window == nullptr)
		fatal("SDL window creation failed: %s", SDL_GetError());

	sdlctx.glcont = SDL_GL_CreateContext(sdlctx.window);
	if (sdlctx.glcont == nullptr)
		fatal("SDL glcont creation failed: %s", SDL_GetError());

	glewExperimental = true;
	if (glewInit() != GLEW_OK)
		fatal("GLEW init failed");

	sdlctx.renderer = SDL_CreateRenderer(sdlctx.window, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

	if (SDL_GL_SetSwapInterval(-1) == -1) {
		warning("Failed to set adaptive vsync, setting regular");
		SDL_GL_SetSwapInterval(1);
	}

#if 0
	auto callback = [] (GLenum src, GLenum type, GLuint id,
			GLenum severity, GLsizei len,
			const char* msg, const void* param)
		-> void { warning ("%s\n", msg); };
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(callback, nullptr);
#endif

	glEnable(GL_MULTISAMPLE);

	extern void init_cuboid ();
	extern void init_text ();
	extern void init_debug ();

	init_cuboid();
	init_debug();
	init_materials();
	init_text();
	init_vis();
	init_gbuffers();
	init_lighting();
	init_sky();
}

void resize_window (int w, int h)
{
	if (w <= 0 || h <= 0) {
		warning("Tried to set window size to %i, %i", w, h);
		return;
	}

	sdlctx.res_x = w;
	sdlctx.res_y = h;
	camera.aspect = (float) w / h;

	sspace_resize_buffers(w, h);

	SDL_SetWindowSize(sdlctx.window, w, h);
}

COMMAND_ROUTINE (windowsize)
{
	if (ev != PRESS)
		return;
	if (args.size() != 2)
		return;

	using std::atoi;
	resize_window(atoi(args[0].c_str()), atoi(args[1].c_str()));
}


GLuint cuboid_dlist_inwards;
GLuint cuboid_dlist_outwards;
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

	cuboid_dlist_inwards = glGenLists(2);
	cuboid_dlist_outwards = cuboid_dlist_inwards + 1;

	glNewList(cuboid_dlist_inwards, GL_COMPILE);
	glBegin(GL_QUADS);
	quad(1, 3, 2, 0);
	quad(4, 5, 1, 0);
	quad(2, 6, 4, 0);
	quad(3, 1, 5, 7);
	quad(6, 2, 3, 7);
	quad(5, 4, 6, 7);
	glEnd();
	glEndList();

	glNewList(cuboid_dlist_outwards, GL_COMPILE);
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


GLuint text_texture;
unsigned int text_program;
unsigned int text_prg_glyph_loc;

void init_text ()
{
	if (TTF_Init() < 0)
		fatal("TTF init failed");
	sdlctx.font = TTF_OpenFont(sdlctx.font_path, sdlctx.font_h);
	if (sdlctx.font == nullptr)
		fatal("Failed to find font %s", sdlctx.font_path);
	if (!TTF_FontFaceIsFixedWidth(sdlctx.font)) {
		warning("Font %s is not monospace. Text will break",
				sdlctx.font_path);
	}
	TTF_GlyphMetrics(sdlctx.font, '~', nullptr, nullptr,
			nullptr, nullptr, &sdlctx.font_w);

	text_program = make_glsl_program(
			{ get_vert_shader("internal/text"),
			  get_frag_shader("internal/text") });
	text_prg_glyph_loc = glGetUniformLocation(text_program, "glyphs");

	char all_chars[257];
	all_chars[0] = '~';
	all_chars[256] = 0;
	for (int i = 1; i < 256; i++)
		all_chars[i] = i;

	SDL_Surface* surf = TTF_RenderText_Blended(sdlctx.font,
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
	int w = sdlctx.font_w;
	int h = sdlctx.font_h;

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


static GLuint debug_program;
static GLuint debug_loc_xy_size;

void init_debug ()
{
	debug_program = make_glsl_program(
		{ get_vert_shader("internal/debug"),
		  get_frag_shader("internal/debug") });

	glUseProgram(debug_program);
	glUniform1i(glGetUniformLocation(debug_program, "tex"), 0);
	debug_loc_xy_size = glGetUniformLocation(debug_program, "xy_size");
}

void debug_render_texture (GLuint tex, float x, float y, float size)
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glUseProgram(debug_program);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tex);

	glUniform3f(debug_loc_xy_size, x, y, size);

	glBegin(GL_QUADS);
	glVertex2i(0, 0);
	glVertex2i(0, 1);
	glVertex2i(1, 1);
	glVertex2i(1, 0);
	glEnd();
}
