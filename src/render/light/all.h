#ifndef ALL_LIGHTING_H
#define ALL_LIGHTING_H

#include "render/framebuffer.h"
#include "misc.h"

/*
 * The common core for lighting. After all, any light will do
 * roughly the same thing regarding the usage of screenspace buffers
 */

void init_lighting ();
void compute_all_lighting ();

/*
 * Called once per material creation - sets the uniforms that do
 * not change between frames, i.e. texture units
 */
void light_init_material ();
/* Called on each material application - binds the textures */
void light_apply_material ();

extern t_fbo sspace_fbo[2];
extern int current_sspace_fbo;

extern vec3 light_ambience;

/*
 * GLSL uniform location for *generating* the light maps
 * Any screenspace light shader should have these declared at these locations
 */
namespace uniform_loc_light
{
	/*
	 * For doing the "ping-pong" accumulation, sampling
	 * the previous buffer while writing to the new one.
	 * TODO: image load/store may be more efficient?
	 */
	constexpr int prev_diffuse_map = 0;
	constexpr int prev_specular_map = 1;

	/*
	 * The sampler for light mapping.
	 * Each light type implements its own, actually
	 */
	constexpr int depth_map = 2;

	/* For specular highlights */
	constexpr int eye_position = 3;
}

/*
 * GLSL uniform locations for *using* light in a material
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
