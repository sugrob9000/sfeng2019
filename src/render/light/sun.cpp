#include "render/light/all.h"
#include "render/light/sun.h"
#include "render/framebuffer.h"
#include "render/resource.h"
#include "render/ctx.h"
#include "render/render.h"
#include "render/vis.h"
#include "render/gbuffer.h"
#include "input/cmds.h"
#include <array>
#include <vector>

/*
 * Quite a bit of the math, matrix work especially,
 * has been trial-and-error'ed until it looked right.
 * There are probably several mistakes that cancel each other out
 */

std::vector<e_light_sun*> lights_sun;

constexpr int sun_num_cascades = 3;
std::array<float, sun_num_cascades + 1> cascade_depths;

static GLuint program;

constexpr int sun_lspace_resolution = 2048;

t_fbo sun_lspace_fbo;

void init_lighting_sun ()
{
	int s = sun_lspace_resolution;
	sun_lspace_fbo.make().attach_depth(make_rbo(s, s, GL_DEPTH_COMPONENT));
	t_attachment* dm = make_tex2d_array(s, s, sun_num_cascades, GL_R32F);
	for (int i = 0; i < sun_num_cascades; i++)
		sun_lspace_fbo.attach_color(dm, i, i);

	program = make_glsl_program(
		{ get_vert_shader("internal/gbuffer_quad"),
		  get_frag_shader("internal/light/sun") });
	glUseProgram(program);

	glUniform1i(uniform_loc_light_sun::depth_map, 2);

	glUniform1i(uniform_loc_gbuffer::world_pos, 3);
	glUniform1i(uniform_loc_gbuffer::world_norm, 4);
	glUniform1i(uniform_loc_gbuffer::specular, 5);
	glUniform1i(uniform_loc_gbuffer::screen_depth, 6);
}


static vec3 unif_rgb;
static mat4 unif_view[sun_num_cascades];
static vec3 unif_direction;
static float unif_depths[sun_num_cascades + 1];

static void fill_depth_maps (const e_light_sun* l)
{
	mat3 rot = rotate_xyz(glm::radians(l->ang - vec3(90.0, 0.0, 0.0)));
	vec3 planes[4 * (sun_num_cascades + 1)];

	for (int i = 0; i < sun_num_cascades + 1; i++) {
		camera.get_corner_points(cascade_depths[i], planes + 4*i);

		// get the depth value while we're at it
		vec4 v = render_ctx.proj * render_ctx.view *
				vec4(planes[4*i], 1.0);

		// so * 0.5 + 0.5 is almost definitely not
		// the right way to do this, but it seems to get
		// all the boundaries *almost* right
		unif_depths[i] = v.z / v.w * 0.5 + 0.5;

		for (int j = 0; j < 4; j++)
			planes[4*i + j] = rot * planes[4*i + j];
	}

	sun_lspace_fbo.apply();
	glClearColor(0.0, 0.0, 0.0, 1.0);

	material_barrier();

	restorer rest(render_ctx);
	render_ctx.view = rot;
	render_ctx.model = mat4(1.0);

	for (unsigned int casc = 0; casc < sun_num_cascades; casc++) {
		t_bound_box lbound = { vec3(INFINITY), vec3(-INFINITY) };
		for (int j = 0; j < 8; j++)
			lbound.expand(planes[4*casc + j]);

		// snap the lightspace bounds so they really only change once
		// in a while during movement. this reduces flicker and gives
		// a bit of leeway for the calculations
		for (int j = 0; j < 3; j++) {
			// round more on farther cascades
			const float step = 15.0 * (casc + 1);
			lbound.start[j] = floor_step(lbound.start[j], step);
			lbound.end[j] = ceil_step(lbound.end[j], step);
		}

		render_ctx.proj = glm::ortho(
			lbound.start.x, lbound.end.x,
			lbound.start.y, lbound.end.y,
			-lbound.start.z - l->distance, -lbound.start.z);
		unif_view[casc] = render_ctx.proj * render_ctx.view;

		sun_lspace_fbo.set_mrt_slots({ GL_COLOR_ATTACHMENT0 + casc });
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		all_leaves.render();
	}

	unif_rgb = l->rgb;
	unif_direction = glm::transpose(rot) * vec3(0.0, 0.0, 1.0);
}

static void lighting_pass ()
{
	sspace_fbo.apply();
	glUseProgram(program);

	// color[0] really could have been any other index
	bind_to_slot(2, GL_TEXTURE_2D_ARRAY, sun_lspace_fbo.color[0]->id);
	bind_tex2d_to_slot(3, gbuf_fbo.color[GBUF_SLOT_WORLD_POS]->id);
	bind_tex2d_to_slot(4, gbuf_fbo.color[GBUF_SLOT_WORLD_NORM]->id);
	bind_tex2d_to_slot(5, gbuf_fbo.color[GBUF_SLOT_SPECULAR]->id);
	bind_tex2d_to_slot(6, gbuf_fbo.depth->id);

	using glm::value_ptr;
	using namespace uniform_loc_light_sun;
	glUniform3fv(light_rgb, 1, value_ptr(unif_rgb));
	glUniformMatrix4fv(light_view, sun_num_cascades, false,
		value_ptr(unif_view[0]));
	glUniform3fv(light_dir, 1, value_ptr(unif_direction));
	glUniform1fv(view_depths, sun_num_cascades + 1, unif_depths);

	glUniform3fv(uniform_loc_light::eye_position,
			1, value_ptr(camera.pos));

	{
		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		gbuffer_pass();
		glDisable(GL_BLEND);
	}
}

void compute_lighting_sun ()
{
	cascade_depths[0] = camera.z_near;
	cascade_depths[sun_num_cascades] = camera.z_far;

	render_ctx.stage = RENDER_STAGE_LIGHTING_LSPACE;

	for (e_light_sun* l: lights_sun) {
		fill_depth_maps(l);
		lighting_pass();
	}
}

COMMAND_ROUTINE (light_cascades)
{
	if (ev != PRESS || args.size() != sun_num_cascades - 1)
		return;

	float new_depths[sun_num_cascades-1];
	for (int i = 0; i < sun_num_cascades-1; i++) {
		new_depths[i] = atof(args[i].c_str());
		if (new_depths[i] < camera.z_near)
			return;
	}

	for (int i = 0; i < sun_num_cascades-1; i++)
		cascade_depths[i + 1] = new_depths[i];
}
