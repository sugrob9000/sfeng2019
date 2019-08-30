#ifndef INC_H
#define INC_H

// in case someone writes g++ ... -DEBUG
#ifdef EBUG
	#define DEBUG
#endif

#ifdef LINUX
	#include <SDL2/SDL.h>
	#include <GL/glew.h>
	#include <SDL2/SDL_opengl.h>
	#include <GL/glu.h>
#elif WINDOWS
	#define SDL_MAIN_HANDLED
	#define GLEW_STATIC
	#include "SDL2/SDL.h"
	#include "GL/glew.h"
	#include "SDL2/SDL_opengl.h"
	#include "GL/glext.h"
#else
	#error Unsupported system or building the wrong way
#endif

#include <iostream>
#include <vector>

#endif // INC_H
