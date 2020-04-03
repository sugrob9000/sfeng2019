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
 * Directional light i.e. sun
 */
class e_light_sun: public e_base
{
	public:

	vec3 rgb;

	/*
	 * Consider the sun to be this far from the player.
	 * If this is closer than anything else on the map,
	 * that will not cast shadows
	 */
	float distance;
	/* Position is mostly irrelevant */

	e_light_sun ();
	~e_light_sun ();

	ENT_MEMBERS (light_sun);
};

#endif // ENT_LIGHTS_H
