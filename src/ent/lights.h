#ifndef ENT_LIGHTS_H
#define ENT_LIGHTS_H

#include "core/entity.h"
#include "render/render.h"
#include "render/vis.h"

/*
 * Directional cone light
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
};

#endif // ENT_LIGHTS_H
