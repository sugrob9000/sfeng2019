#include "render/light/all.h"
#include "render/light/cone.h"
#include "render/light/sun.h"

#include "render/render.h"
#include "render/resource.h"
#include "render/framebuffer.h"
#include "render/gbuffer.h"
#include "input/cmds.h"
#include "misc.h"

t_fbo sspace_fbo;
vec3 light_ambience;

/* TODO: do anything useful in post-processing */

void init_lighting ()
{
	int w = sdlctx.res_x;
	int h = sdlctx.res_y;
	constexpr GLenum f = GL_R11F_G11F_B10F;

	sspace_fbo.make()
		.attach_color(make_tex2d(w, h, f), LIGHT_SLOT_DIFFUSE)
		.attach_color(make_tex2d(w, h, f), LIGHT_SLOT_SPECULAR)
		.assert_complete();
	sspace_add_buffer(sspace_fbo);

	init_lighting_cone();
	init_lighting_sun();
}

void compute_all_lighting ()
{
	sspace_fbo.apply();

	glClearColor(light_ambience.x,
	             light_ambience.y,
	             light_ambience.z, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	// go through all the kinds of lights
	compute_lighting_cone();
	compute_lighting_sun();
}

void light_init_material ()
{
	using namespace uniform_loc_light_material;
	glUniform1i(lightmap_diffuse, 0);
	glUniform1i(lightmap_specular, 1);
}

void light_apply_material ()
{
	bind_tex2d_to_slot(0, sspace_fbo.color[LIGHT_SLOT_DIFFUSE]->id);
	bind_tex2d_to_slot(1, sspace_fbo.color[LIGHT_SLOT_SPECULAR]->id);
}


COMMAND_ROUTINE (light_ambience)
{
	if (ev != PRESS)
		return;
	switch (args.size()) {
	case 1:
		light_ambience = vec3(atof(args[0].c_str()));
		break;
	case 3:
		light_ambience = vec3(atof(args[0].c_str()),
		                      atof(args[1].c_str()),
		                      atof(args[2].c_str()));
		break;
	default:
		return;
	}
}
