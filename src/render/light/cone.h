#ifndef LIGHT_CONE_H
#define LIGHT_CONE_H

#include "render/render.h"
#include "ent/lights.h"
#include <vector>

void init_lighting_cone ();

void compute_lighting_cone ();

extern std::vector<e_light_cone*> cone_lights;

/*
 * GLSL uniform locations for calculating light
 * when rendering actual geometry from a light's perspective
 */
constexpr int UNIFORM_LOC_DEPTH_MAP = 0;
constexpr int UNIFORM_LOC_EYE_POSITION = 3;

constexpr int UNIFORM_LOC_LIGHT_POS = 6;
constexpr int UNIFORM_LOC_LIGHT_RGB = 9;
constexpr int UNIFORM_LOC_LIGHT_VIEW = 12;
constexpr int UNIFORM_LOC_LIGHT_BOUNDS = 100;

/*
 * GLSL uniform locations for using light
 * when rendering screenspace buffers
 */
constexpr int UNIFORM_LOC_LIGHTMAP_DIFFUSE = 1;
constexpr int UNIFORM_LOC_LIGHTMAP_SPECULAR = 2;

/*
 * Which texture units to bind the respective buffers to
 */
constexpr int TEXTURE_SLOT_PREV_SHADOWMAP = 0;
constexpr int TEXTURE_SLOT_DEPTH_MAP = 1;

#endif // LIGHT_CONE_H
