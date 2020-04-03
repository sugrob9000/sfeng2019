#include "render/light/cone.h"
#include "render/light/all.h"
#include "render/material.h"
#include "render/resource.h"
#include "render/gbuffer.h"
#include "render/ctx.h"
#include "core/core.h"
#include "misc.h"

std::vector<e_light_cone*> lights_cone;

/* Sending lighting info to shader */
static vec3 unif_pos;
static vec3 unif_rgb;
static mat4 unif_view;
static float unif_bounds[5]; /* X_low, Y_low, X_high, Y_high, Z_high */

constexpr int cone_lspace_resolution = 1024;
static t_fbo lspace_fbo;

static GLuint program;

void init_lighting_cone ()
{
	int s = cone_lspace_resolution;
	lspace_fbo.make()
		.attach_color(make_tex2d(s, s, GL_R32F))
		.attach_depth(make_rbo(s, s, GL_DEPTH_COMPONENT))
		.assert_complete();

	program = make_glsl_program(
		{ get_vert_shader("internal/gbuffer_quad"),
		  get_frag_shader("internal/light_cone") });
	glUseProgram(program);

	glUniform1i(UNIFORM_LOC_PREV_DIFFUSE_MAP, 0);
	glUniform1i(UNIFORM_LOC_PREV_SPECULAR_MAP, 1);

	glUniform1i(uniform_loc_light_cone::depth_map, 2);

	using namespace uniform_loc_gbuffer;
	glUniform1i(world_pos, 3);
	glUniform1i(world_norm, 4);
	glUniform1i(specular, 5);
	glUniform1i(screen_depth, 6);
}

/* Returns: whether this light is potentially visible */
static bool fill_depth_map (const e_light_cone* l)
{
	static t_bound_box view_bounds =
		{ { -1.0, -1.0, 0.0 }, { 1.0, 1.0, 1.0 } };

	// aliases
	mat4& proj = render_ctx.proj;
	mat4& view = render_ctx.view;

	vec4 camspace = proj * view * vec4(l->pos, 1.0);
	t_bound_box lbounds;
	if (view_bounds.point_in(camspace / camspace.w)) {
		// cannot cull XY when light is visible on screen
		lbounds = { { -1.0, -1.0, 0.0 }, { 1.0, 1.0, 0.0 } };
	} else {
		lbounds = { vec3(INFINITY), vec3(-INFINITY) };
	}

	matrix_restorer rest(render_ctx);
	l->view();

	std::array<vec3, 4> planes[2] =
		{ camera.corner_points(camera.z_far),
		  camera.corner_points(camera.z_near) };
	for (int i = 0; i < 8; i++) {
		const vec3& worldspace = planes[i / 4][i % 4];
		vec4 lightspace = proj * view * vec4(worldspace, 1.0);
		lbounds.expand(lightspace / std::abs(lightspace.w));
	}

	// clip to what the light can see
	lbounds.intersect_guarded(view_bounds);

	if (lbounds.volume() <= 0.0) {
		// we cannot see this light at all
		return false;
	}

	// make sure to draw everything between light and slice, too
	lbounds.start.z = 0.0;

	unif_view = proj * view;
	unif_pos = l->pos;
	unif_rgb = l->rgb;
	unif_bounds[0] = lbounds.start.x;
	unif_bounds[1] = lbounds.start.y;
	unif_bounds[2] = lbounds.end.x;
	unif_bounds[3] = lbounds.end.y;
	unif_bounds[4] = lbounds.end.z;

	// do the rendering at the intersection
	// of what light sees and what we see
	vec3 center = vec3(-0.5, -0.5, 0.0) * (lbounds.start + lbounds.end);
	vec3 scale = vec3(2.0, 2.0, 1.0) / (lbounds.end - lbounds.start);
	mat4 subfrustum = glm::translate(glm::scale(mat4(1.0), scale), center);
	render_ctx.proj = subfrustum * render_ctx.proj;

	lspace_fbo.apply();
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	material_barrier();
	l->vis.render();

	return true;
}

static void lighting_pass ()
{
	current_sspace_fbo ^= 1;
	sspace_fbo[current_sspace_fbo].apply();

	glUseProgram(program);

	const t_fbo& other_fbo = sspace_fbo[current_sspace_fbo ^ 1];
	bind_tex2d_to_slot(0, other_fbo.color[LIGHT_SLOT_DIFFUSE]->id);
	bind_tex2d_to_slot(1, other_fbo.color[LIGHT_SLOT_SPECULAR]->id);

	bind_tex2d_to_slot(2, lspace_fbo.color[0]->id);

	bind_tex2d_to_slot(3, gbuf_fbo.color[GBUF_SLOT_WORLD_POS]->id);
	bind_tex2d_to_slot(4, gbuf_fbo.color[GBUF_SLOT_WORLD_NORM]->id);
	bind_tex2d_to_slot(5, gbuf_fbo.color[GBUF_SLOT_SPECULAR]->id);
	bind_tex2d_to_slot(6, gbuf_fbo.depth->id);

	using glm::value_ptr;
	using namespace uniform_loc_light_cone;
	glUniform3fv(light_pos, 1, value_ptr(unif_pos));
	glUniform3fv(light_rgb, 1, value_ptr(unif_rgb));
	glUniformMatrix4fv(light_view, 1, false, value_ptr(unif_view));
	glUniform1fv(light_bounds, 5, unif_bounds);

	glUniform3fv(eye_position, 1, value_ptr(camera.pos));

	gbuffer_pass();
}

void compute_lighting_cone ()
{
	render_ctx.stage = RENDER_STAGE_LIGHTING_LSPACE;
	for (e_light_cone* l: lights_cone) {
		if (fill_depth_map(l))
			lighting_pass();
	}
}
