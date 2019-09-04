#include "inc_general.h"
#include "inc_gl.h"
#include "render.h"

namespace render
{

int resolution_x;
int resolution_y;

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

	resolution_x = resx;
	resolution_y = resy;

	window = SDL_CreateWindow("Engine",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			resolution_x, resolution_y,
			SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

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

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glEnable(GL_DEPTH_TEST);

	return true;
}

}

