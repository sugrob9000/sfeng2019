#ifndef SHADERLIB_H
#define SHADERLIB_H

#include "inc_gl.h"

/*
 * Uniforms for lighting. Must match what the
 * actual locations in lib shaders!
 */
constexpr float UNIFORM_LOC_PREV_SHADOWMAP = 1;
constexpr float UNIFORM_LOC_DEPTH_MAP = 2;
constexpr float UNIFORM_LOC_LIGHT_POS = 3;
constexpr float UNIFORM_LOC_LIGHT_RGB = 6;
constexpr float UNIFORM_LOC_LIGHT_VIEWMAT = 9;

/*
 * Uniform for the current render stage
 */
constexpr int UNIFORM_LOC_RENDER_STAGE = 0;

/*
 * Attribute for the tangent vector (TBN matrix calculation)
 */
constexpr GLuint ATTRIB_LOC_TANGENT = 1;

#endif // SHADERLIB_H
