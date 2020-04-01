#include "render/light/all.h"
#include "render/light/cone.h"
#include "render/render.h"
#include "input/cmds.h"
#include "misc.h"

t_fbo sspace_fbo[2];
int current_sspace_fbo;

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

	// go through all the kinds of lights
	compute_lighting_cone();
}

vec3 light_ambience;
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
