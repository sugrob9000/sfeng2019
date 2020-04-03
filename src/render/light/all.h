#ifndef ALL_LIGHTING_H
#define ALL_LIGHTING_H

#include "render/framebuffer.h"
#include "misc.h"

/*
 * The collective file for all light types
 *  - init them all, compute them all, etc.
 *
 * Note: so far there is only one light type, the cone light.
 * TODO: omnidirectional light and a "sun" (single-directional everywhere)
 */

void init_lighting ();
void compute_all_lighting ();

/* Called once per material creation - sets the uniforms */
void light_init_material ();
/* Called on each material application - binds the textures */
void light_apply_material ();

extern t_fbo sspace_fbo[2];
extern int current_sspace_fbo;

extern vec3 light_ambience;

constexpr int UNIFORM_LOC_PREV_DIFFUSE_MAP = 0;
constexpr int UNIFORM_LOC_PREV_SPECULAR_MAP = 1;

/*
 * GLSL uniform locations for using light
 * when rendering screenspace buffers
 */
namespace uniform_loc_light_material
{
	constexpr int lightmap_diffuse = 1;
	constexpr int lightmap_specular = 2;
}

/* Framebuffer slots for light MRT */
constexpr int LIGHT_SLOT_DIFFUSE = 0;
constexpr int LIGHT_SLOT_SPECULAR = 1;

#endif // ALL_LIGHTING_H
