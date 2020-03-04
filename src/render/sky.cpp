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

static const t_bound_box box = { vec3(-1.0), vec3(1.0) };
void render_sky ()
{
	material_barrier();
	glUseProgram(program);

	glDisable(GL_DEPTH_TEST);
	glFrontFace(GL_CW);

	glPushMatrix();
	translate_gl_matrix(camera.pos);
	draw_cuboid(box);
	glPopMatrix();

	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);
}

