#include "render/light/all.h"
#include "render/light/sun.h"
#include "render/framebuffer.h"
#include "render/resource.h"
#include "render/ctx.h"
#include <array>
#include <vector>

std::vector<e_light_sun*> lights_sun;

constexpr int num_cascades = 4;

static vec3 unif_rgb;
static vec3 unif_view;
static float unif_bounds[5 * num_cascades];

static GLuint program;

constexpr int sun_lspace_resolution = 1024;
constexpr int sun_num_cascades = 4;

static t_fbo lspace_fbo;

void init_lighting_sun ()
{
	int s = sun_lspace_resolution;
	lspace_fbo.make().attach_depth(make_rbo(s, s, GL_DEPTH_COMPONENT));
	t_attachment* dm = make_tex2d_array(s, s, sun_num_cascades, GL_R32F);
	for (int i = 0; i < sun_num_cascades; i++)
		lspace_fbo.attach_color(dm, i, i);

	program = make_glsl_program(
		{ get_vert_shader("internal/gbuffer_quad"),
		  get_frag_shader("internal/light/sun") });
	glUseProgram(program);

	glUniform1i(uniform_loc_light::prev_diffuse_map, 0);
	glUniform1i(uniform_loc_light::prev_specular_map, 1);
}


void compute_lighting_sun ()
{
	render_ctx.stage = RENDER_STAGE_LIGHTING_LSPACE;

	// TODO
}
