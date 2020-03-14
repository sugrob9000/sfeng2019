#include "render/sky.h"
#include "render/render.h"
#include "render/resource.h"
#include "render/material.h"
#include "camera.h"

static GLuint program;

void init_sky ()
{
	program = make_glsl_program(
		{ get_vert_shader("lib/sky"),
		  get_frag_shader("lib/sky") });
}

void render_sky ()
{
	material_barrier();
	glUseProgram(program);
	render_ctx.submit_viewproj();

	glDisable(GL_DEPTH_TEST);
	glCallList(cuboid_dlist_inwards);
	glEnable(GL_DEPTH_TEST);
}

