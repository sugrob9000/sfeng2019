#include "render/light/sun.h"
#include <array>
#include <vector>

std::vector<e_light_sun*> lights_sun;

constexpr int num_cascades = 4;

static vec3 unif_rgb;
static vec3 unif_view;
static float unif_bounds[5 * num_cascades];

constexpr int sun_lspace_resolution = 1024;
static t_fbo lspace_fbo;

static GLuint program;


void compute_lighting_sun ()
{
	render_ctx.stage = RENDER_LIGHTING_LSPACE;

	for (e_light_sun* l: lights_sun) {
		if (fill_depth_maps(l))
			lighting_pass();
	}
}
