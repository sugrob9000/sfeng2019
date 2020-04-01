#ifndef ENT_LIGHTS_H
#define ENT_LIGHTS_H

#include "core/entity.h"
#include "render/render.h"
#include "render/vis.h"

/*
 * Directional light
 */
class e_light: public e_base
{
	public:

	float cone_angle;
	float near_plane;
	float reach;
	vec3 rgb;

	t_visible_set vis;

	e_light ();

	ENT_MEMBERS (light);
	virtual void moved ();

	void view () const;

	/* Sending lighting info to shader */
	static vec3 unif_pos;
	static vec3 unif_rgb;
	static mat4 unif_view;
	static vec2 unif_cascade_bounds[2];
};

#endif // ENT_LIGHTS_H
