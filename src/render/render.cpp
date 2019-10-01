#include "inc_general.h"
#include "inc_gl.h"
#include "render/render.h"

namespace render
{

SDL_Window* window;
SDL_GLContext context;

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

	return true;
}

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
	// TODO
}

}

