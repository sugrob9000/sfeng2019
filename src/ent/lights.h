#ifndef ENT_LIGHTS_H
#define ENT_LIGHTS_H

#include "core/entity.h"
#include "render/render.h"
#include "render/vis.h"

/*
 * Directional cone light
 */
class e_light_cone: public e_base
{
	public:

	float cone_angle;
	float near_plane;
	float reach;
	vec3 rgb;

	t_visible_set vis;

	e_light_cone ();
	~e_light_cone ();

	ENT_MEMBERS (light_cone);
	virtual void moved ();

	void view () const;
};

/*
 * Parallel omnipresent light i.e. a sun
 * World position on this doesn't change anything about its lighting
 */
class e_light_sun: public e_base
{
	public:

	vec3 rgb;

	/*
	 * Consider the sun to be this far from the eye, in the (opposite)
	 *   direction of the rays. If there's something in the map farther
	 *   than this, it will not cast shadows
	 * Smaller values mean better depth buffer precision, though
	 */
	float distance;

	e_light_sun ();
	~e_light_sun ();

	ENT_MEMBERS (light_sun);
};

#endif // ENT_LIGHTS_H
