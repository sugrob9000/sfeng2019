#include "inc_general.h"
#include "inc_gl.h"
#include "render/render.h"
#include "core/core.h"
#include "input/input.h"

namespace render
{

SDL_Window* window;
SDL_GLContext context;
t_camera camera;

void render_all ()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	camera.apply();

	// TODO: visible sets
	for (const core::e_base* e: core::game.ents.v)
		e->render();

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glUseProgram(0);

	SDL_GL_SwapWindow(window);
}

bool cam_move_flags[4];

bool init (int resx, int resy)
{
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		printf("SDL_Init failed: %s\n", SDL_GetError());
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
	                    SDL_GL_CONTEXT_PROFILE_CORE);

	window = SDL_CreateWindow("Engine",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			resx, resy, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	if (window == nullptr) {
		printf("SDL window creation failed: %s\n", SDL_GetError());
		return false;
	}

	context = SDL_GL_CreateContext(window);

	if (context == nullptr) {
		printf("SDL context creation failed: %s\n", SDL_GetError());
		return false;
	}

	glewExperimental = true;
	if (glewInit() != GLEW_OK)
		return false;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	return true;
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
	glRotatef(ang.x, 1.0, 0.0, 0.0);
	glRotatef(ang.y, 0.0, 1.0, 0.0);
	glRotatef(ang.z, 0.0, 0.0, 1.0);
	glTranslatef(-pos.x, -pos.y, -pos.z);
}

}
