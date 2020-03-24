#ifndef ENT_LIGHTS_H
#define ENT_LIGHTS_H

#include "core/entity.h"
#include "render/render.h"
#include "render/vis.h"
#include "render/framebuffer.h"

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
	static mat4 uniform_view;
	static vec3 uniform_pos;
	static vec3 uniform_rgb;
};

extern std::vector<e_light*> lights;

constexpr float LIGHT_Z_NEAR = 10.0;
extern vec3 ambient;

extern t_fbo lspace_fbo;
extern t_fbo lspace_fbo_ms;

void light_apply_uniforms ();

void init_lighting ();
void compute_lighting ();

#endif // ENT_LIGHTS_H
