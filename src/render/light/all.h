#ifndef ALL_LIGHTING_H
#define ALL_LIGHTING_H

#include "render/framebuffer.h"
#include "render/light/cone.h"
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

/*
 * This is run *once* at material's creation
 */
inline void light_init_material ()
{
	light_cone_init_material_uniforms();
}

/*
 * This is run *every time* the material is applied
 */
inline void light_apply_material ()
{
	light_cone_apply_material_uniforms();
}

extern t_fbo sspace_fbo[2];
extern int current_sspace_fbo;

extern vec3 light_ambience;

#endif // ALL_LIGHTING_H
