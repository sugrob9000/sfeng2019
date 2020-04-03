#include "render/light/all.h"
#include "render/light/cone.h"
#include "render/render.h"
#include "render/resource.h"
#include "render/framebuffer.h"
#include "render/gbuffer.h"
#include "input/cmds.h"
#include "misc.h"

t_fbo sspace_fbo[2];
int current_sspace_fbo;
vec3 light_ambience;

/* TODO: do anything useful in post-processing */

void init_lighting ()
{
	int w = sdlctx.res_x;
	int h = sdlctx.res_y;
	constexpr GLenum f = GL_R11F_G11F_B10F;

	for (int i: { 0, 1 }) {
		sspace_fbo[i].make()
			.attach_color(make_tex2d(w, h, f), LIGHT_SLOT_DIFFUSE)
			.attach_color(make_tex2d(w, h, f), LIGHT_SLOT_SPECULAR)
			.assert_complete();
		sspace_add_buffer(sspace_fbo[i]);
	}

	init_lighting_cone();
}

void compute_all_lighting ()
{
	current_sspace_fbo = 0;
	sspace_fbo[0].apply();
	glClearColor(light_ambience.x, light_ambience.y,
			light_ambience.z, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);

	// go through all the kinds of lights (there is only one yet)
	compute_lighting_cone();
}

void light_init_material ()
{
	using namespace uniform_loc_light_material;
	glUniform1i(lightmap_diffuse, 0);
	glUniform1i(lightmap_specular, 1);
}

void light_apply_material ()
{
	const t_fbo& sspace = sspace_fbo[current_sspace_fbo];
	bind_tex2d_to_slot(0, sspace.color[LIGHT_SLOT_DIFFUSE]->id);
	bind_tex2d_to_slot(1, sspace.color[LIGHT_SLOT_SPECULAR]->id);
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
