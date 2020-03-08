#ifndef INC_GL_H
#define INC_GL_H

#ifdef LINUX
	#include <SDL2/SDL.h>
	#include <GL/glew.h>
	#include <SDL2/SDL_opengl.h>
	#include <SDL2/SDL_image.h>
	#include <SDL2/SDL_ttf.h>
	#include <GL/glu.h>
#elif WINDOWS
	#define SDL_MAIN_HANDLED
	#define GLEW_STATIC
	#include "SDL2/SDL.h"
	#include "GL/glew.h"
	#include "SDL2/SDL_opengl.h"
	#include "SDL2/SDL_image.h"
	#include "SDL2/SDL_ttf.h"
	#include <GL/glu.h>
	#include "GL/glext.h"
#else
	#error "Neither WINDOWS nor LINUX defined"
#endif

/*
 * Uniform and attribute locations. Must match the explicit
 * locations specified in the respective lib shaders!
 */

/* Main matrices */
constexpr int UNIFORM_LOC_PROJ = 100;
constexpr int UNIFORM_LOC_VIEW = 116;
constexpr int UNIFORM_LOC_MODEL = 132;

/* The current render stage */
constexpr int UNIFORM_LOC_RENDER_STAGE = 0;

/* Lighting data */
constexpr int UNIFORM_LOC_PREV_SHADOWMAP = 1;
constexpr int UNIFORM_LOC_DEPTH_MAP = 10;
constexpr int UNIFORM_LOC_EYE_POSITION = 11;
constexpr int UNIFORM_LOC_LIGHT_POS = 12;
constexpr int UNIFORM_LOC_LIGHT_RGB = 15;
constexpr int UNIFORM_LOC_LIGHT_VIEW = 18;

/* The tangent vector (TBN matrix calculation) */
constexpr GLuint ATTRIB_LOC_TANGENT = 1;

/* Vis cuboids */
constexpr GLuint UNIFORM_LOC_VIS_CUBE = 42;

/* Lighting: Which texture units to bind the respective buffers to */
constexpr int TEXTURE_SLOT_PREV_SHADOWMAP = 0;
constexpr int TEXTURE_SLOT_DEPTH_MAP = 1;


#endif // INC_GL_H
