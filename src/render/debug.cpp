#include "render/render.h"
#include "render/resource.h"
#include "render/debug.h"

static constexpr int UNIFORM_LOC_XY_SIZE = 0;
static constexpr int UNIFORM_LOC_TEX = 3;
static constexpr int UNIFORM_LOC_LAYER = 4;

static GLuint prog_tex2d;
static GLuint prog_tex2d_array;

void init_render_debug ()
{
	prog_tex2d = make_glsl_program(
		{ get_vert_shader("internal/debug/screen_quad"),
		  get_frag_shader("internal/debug/tex2d") });
	glUseProgram(prog_tex2d);
	glUniform1i(UNIFORM_LOC_TEX, 0);

	prog_tex2d_array = make_glsl_program(
		{ get_vert_shader("internal/debug/screen_quad"),
		  get_frag_shader("internal/debug/tex2d_array") });
	glUseProgram(prog_tex2d_array);
	glUniform1i(UNIFORM_LOC_TEX, 0);
}

void debug_render_tex2d (GLuint tex, float x, float y, float size)
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glUseProgram(prog_tex2d);
	bind_tex2d_to_slot(0, tex);
	glUniform3f(UNIFORM_LOC_XY_SIZE, x, y, size);

	glBegin(GL_QUADS);
	glVertex2i(0, 0);
	glVertex2i(0, 1);
	glVertex2i(1, 1);
	glVertex2i(1, 0);
	glEnd();
}

void debug_render_tex2d_array (GLuint tex, int layer,
		float x, float y, float size)
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glUseProgram(prog_tex2d_array);
	bind_to_slot(0, GL_TEXTURE_2D_ARRAY, tex);
	glUniform3f(UNIFORM_LOC_XY_SIZE, x, y, size);
	glUniform1ui(UNIFORM_LOC_LAYER, layer);

	glBegin(GL_QUADS);
	glVertex2i(0, 0);
	glVertex2i(0, 1);
	glVertex2i(1, 1);
	glVertex2i(1, 0);
	glEnd();
}
