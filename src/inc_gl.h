#pragma once
#include <GL/glew.h>
#include <GL/glu.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_opengl.h>
#include <SDL2/SDL_ttf.h>

// Uniform and attribute locations. Must match the explicit
// locations specified in the respective lib shaders!

// Main matrices
constexpr int UNIFORM_LOC_PROJ = 100;
constexpr int UNIFORM_LOC_VIEW = 116;
constexpr int UNIFORM_LOC_MODEL = 132;

// The current render stage
constexpr int UNIFORM_LOC_RENDER_STAGE = 0;

// The tangent vector (TBN matrix calculation)
constexpr GLuint ATTRIB_LOC_TANGENT = 1;

// Vis cuboids
constexpr GLuint UNIFORM_LOC_VIS_CUBE = 42;