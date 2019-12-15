#ifndef ENT_LIGHTS_H
#define ENT_LIGHTS_H

#include "core/entity.h"
#include "render/render.h"

/*
 * Directional light
 */
class e_light: public e_base
{
	public:

	float cone_angle;
	float reach;
	vec3 rgb;

	SDL_Color color;

	e_light ();

	ENT_MEMBERS (light)

	/*
	 * These get sent to the shader as one uniform.
	 * Note that we don't need cone_angle or ang in here
	 */
	static float uniform_viewmat[16];
	static vec3 uniform_rgb;
	static vec3 uniform_pos;
};

constexpr float LIGHT_Z_NEAR = 10.0;

/*
 * !!! These must match what is found in lib shaders !!!
 */
constexpr float UNIFORM_LOC_PREV_SHADOWMAP = 1;

constexpr float UNIFORM_LOC_DEPTH_MAP = 2;
constexpr float UNIFORM_LOC_LIGHT_POS = 3;
constexpr float UNIFORM_LOC_LIGHT_RGB = 6;
constexpr float UNIFORM_LOC_LIGHT_VIEWMAT = 9;

const vec3 ambient = { 0.4, 0.4, 0.4 };

void light_apply_uniforms (t_render_stage s);

extern GLuint lspace_fbo_texture;
extern GLuint sspace_fbo_texture[2];
extern int current_sspace_fbo;

extern std::vector<e_light*> lights;

void init_lighting ();
void compute_lighting ();

#endif // ENT_LIGHTS_H
