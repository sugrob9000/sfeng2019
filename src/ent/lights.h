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
	static mat4 unif_view;
	static vec3 unif_pos;
	static vec3 unif_rgb;
};

extern std::vector<e_light*> lights;

constexpr float LIGHT_Z_NEAR = 10.0;
extern vec3 ambient;

extern t_fbo lspace_fbo;
extern t_fbo lspace_fbo_ms;
extern t_fbo sspace_fbo[2];
extern int current_sspace_fbo;

void init_lighting ();
void compute_lighting ();

void light_init_material ();
void light_apply_material ();

/*
 * GLSL uniform locations for calculating light
 */
constexpr int UNIFORM_LOC_PREV_SHADOWMAP = 0;
constexpr int UNIFORM_LOC_DEPTH_MAP = 1;
constexpr int UNIFORM_LOC_EYE_POSITION = 3;

constexpr int UNIFORM_LOC_LIGHT_POS = 6;
constexpr int UNIFORM_LOC_LIGHT_RGB = 9;
constexpr int UNIFORM_LOC_LIGHT_VIEW = 12;

/*
 * GLSL uniform locations for using light - screenspace buffers
 */
constexpr int UNIFORM_LOC_LIGHTMAP_DIFFUSE = 1;
constexpr int UNIFORM_LOC_LIGHTMAP_SPECULAR = 2;

/*
 * Which texture units to bind the respective buffers to
 */
constexpr int TEXTURE_SLOT_PREV_SHADOWMAP = 0;
constexpr int TEXTURE_SLOT_DEPTH_MAP = 1;

#endif // ENT_LIGHTS_H
