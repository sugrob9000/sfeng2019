#include "render/sky.h"
#include "render/render.h"
#include "render/resource.h"
#include "camera.h"

static GLuint program;

void init_sky ()
{
	program = glCreateProgram();
	glAttachShader(program, get_vert_shader("lib/sky"));
	glAttachShader(program, get_frag_shader("lib/sky"));
	glLinkProgram(program);
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

