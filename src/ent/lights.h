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

	/*
	 * These get sent to the shader as one uniform.
	 * Note that we don't need cone_angle or ang in here
	 */
	static float uniform_viewmat[16];
	static vec3 uniform_rgb;
	static vec3 uniform_pos;
};

constexpr float LIGHT_Z_NEAR = 10.0;
const vec3 ambient = vec3(0.15);

void light_apply_uniforms (t_render_stage s);

extern int current_sspace_fbo;
extern t_fbo sspace_fbo[2];
extern t_fbo lspace_fbo;

extern std::vector<e_light*> lights;

void init_lighting ();
void compute_lighting ();

#endif // ENT_LIGHTS_H
