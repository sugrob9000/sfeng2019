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
	#error "Unsupported system or building the wrong way"
#endif

/*
 * We decompose the matrix like
 *   (projection * view), (model)
 * but abuse the existing OpenGL matrices for this, so
 *   projection matrix  is used to store  (projection * view), and
 *   modelview matrix   is used to store   model
 */
constexpr GLenum MTX_VIEWPROJ = GL_PROJECTION;
constexpr GLenum MTX_MODEL = GL_MODELVIEW;
constexpr GLenum MTX_VIEWPROJ_MATRIX = GL_PROJECTION_MATRIX;
constexpr GLenum MTX_MODEL_MATRIX = GL_MODELVIEW_MATRIX;


#endif // INC_GL_H
