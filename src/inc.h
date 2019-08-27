#ifndef INC_H
#define INC_H

#ifdef __linux__
	#include <SDL2/SDL.h>
	#include <GL/glew.h>
	#include <SDL2/SDL_opengl.h>
	#include <GL/glu.h>
#elif _WIN32

#else
	#error "Unsupported system"
#endif

#include <iostream>
#include <vector>

#endif // INC_H
