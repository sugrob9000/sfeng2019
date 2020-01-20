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
	float reach;
	vec3 rgb;

	t_visible_set vis;

	e_light ();

	ENT_MEMBERS (light);
	virtual void moved ();

	void view () const;

	/* Sending lighting info to shader */
	static constexpr int BATCH = 16;
	static float uniform_view[16][BATCH];
	static vec3 uniform_pos[BATCH];
	static vec3 uniform_rgb[BATCH];
};

extern std::vector<e_light*> lights;

constexpr float LIGHT_Z_NEAR = 10.0;
extern vec3 ambient;

void light_apply_uniforms (t_render_stage s);

void init_lighting ();
void compute_lighting ();

#endif // ENT_LIGHTS_H
