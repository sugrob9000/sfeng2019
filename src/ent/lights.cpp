#include "ent/lights.h"
#include "input/cmds.h"
#include "render/framebuffer.h"
#include "render/render.h"
#include "render/resource.h"
#include "render/vis.h"
#include "render/gbuffer.h"

std::vector<e_light*> lights;

/* ============= e_light code ============= */

SIG_HANDLER (light, setcolor)
{
	atovec3(arg, ent->rgb);
}

SIG_HANDLER (light, setcone)
{
	float cone = atof(arg.c_str());
	if (cone > 0.0 && cone < 180.0)
		ent->cone_angle = cone;
}

FILL_IO_DATA (light)
{
	BASIC_SIG_HANDLERS(light);
	SET_SIG_HANDLER(light, setcolor);
	SET_SIG_HANDLER(light, setcone);
}

e_light::e_light ()
{
	lights.push_back(this);
}

void e_light::moved ()
{
	e_base::moved();

	// update visible set
	view();
	vis.fill(pos);
}

void e_light::think () { }

void e_light::apply_keyvals (const t_ent_keyvals& kv)
{
	apply_basic_keyvals(kv);

	KV_TRY_GET(kv["cone"],
		cone_angle = atof(val.c_str());,
		cone_angle = 60.0; );
	KV_TRY_GET(kv["reach"],
		reach = atof(val.c_str());,
		reach = 2000.0; );
	KV_TRY_GET(kv["rgb"],
		atovec3(val, rgb);,
		rgb = vec3(0.5); );
}

void e_light::render () const { }

t_bound_box e_light::get_bbox () const { return { }; }

void e_light::view () const
{
	using namespace glm;

	render_ctx.proj = perspective(radians(2.0f * cone_angle),
			1.0f, LIGHT_Z_NEAR, reach);
	render_ctx.view = rotate_xyz(radians(ang - vec3(90.0, 0.0, 0.0)));
	render_ctx.view = translate(render_ctx.view, -pos);
	render_ctx.model = mat4(1.0);
}



/* ======================================== */

/* Depth maps from lights' perspective */
constexpr int lspace_samples = 4;
constexpr int lspace_resolution = 1024;

/* Render to multisampled, then blit into regular for sampling */
t_fbo lspace_fbo_ms;
t_fbo lspace_fbo;

/* Screen space shadow maps */
t_fbo sspace_fbo[2];
int current_sspace_fbo = 0;

static GLuint lighting_program;

void init_lighting ()
{
	constexpr int s = lspace_resolution;

	lspace_fbo_ms.make()
		.attach_color(make_rbo_msaa(s, s, GL_R16F, lspace_samples))
		.attach_depth(make_rbo_msaa(
			s, s, GL_DEPTH_COMPONENT, lspace_samples))
		.assert_complete();

	lspace_fbo.make()
		.attach_color(make_tex2d(s, s, GL_R16F))
		.assert_complete();

	int sw = sdlctx.res_x;
	int sh = sdlctx.res_y;
	for (int i: { 0, 1 }) {
		sspace_fbo[i].make()
			.attach_color(make_tex2d(sw, sh, GL_RGB16F))
			.assert_complete();
		sspace_add_buffer(sspace_fbo[i]);
	}

	lighting_program = make_glsl_program(
		{ get_vert_shader("internal/gbuffer_quad"),
		  get_frag_shader("internal/light") });

	glUseProgram(lighting_program);
	glUniform1i(UNIFORM_LOC_PREV_SHADOWMAP, 0);
	glUniform1i(UNIFORM_LOC_DEPTH_MAP, 1);
	glUniform1i(UNIFORM_LOC_GBUFFER_WORLD_POS, 2);
	glUniform1i(UNIFORM_LOC_GBUFFER_WORLD_NORM, 3);
}

void light_init_material ()
{
	glUniform1i(UNIFORM_LOC_LIGHTMAP_DIFFUSE, 0);
	glUniform1i(UNIFORM_LOC_LIGHTMAP_SPECULAR, 1);
}

void light_apply_material ()
{
	bind_tex2d_to_slot(0, sspace_fbo[current_sspace_fbo].color[0]->id);
	// TODO: specular
}


mat4 e_light::unif_view;
vec3 e_light::unif_pos;
vec3 e_light::unif_rgb;
t_bound_box e_light::unif_cascade_bounds;


/* Returns: whether this light is actually potentially visible */
static bool fill_depth_map (const e_light* l)
{
	matrix_restorer restore(render_ctx);

	l->view();

	std::array<vec3, 4> planes[2] =
		{ camera.corner_points(camera.z_near),
		  camera.corner_points(camera.z_far) };

	t_bound_box bounds = { vec3(INFINITY), vec3(-INFINITY) };
	for (int i = 0; i < 8; i++) {
		vec4 v = render_ctx.proj * render_ctx.view *
				vec4(planes[i / 4][i % 4], 1.0);
		bounds.expand(v / std::abs(v.w));
	}
	bounds.intersect({ { -1.0, -1.0, 0.0 }, { 1.0, 1.0, 1.0 } });
	if (bounds.volume() <= 0.0) {
		// cannot see this light
		return false;
	}
	// ensure everything between the light and the slice is rendered
	bounds.start.z = 0.0;

	lspace_fbo_ms.apply();
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	l->vis.render();

	glBindFramebuffer(GL_READ_FRAMEBUFFER, lspace_fbo_ms.id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lspace_fbo.id);
	glBlitFramebuffer(0, 0, lspace_resolution, lspace_resolution,
	                  0, 0, lspace_resolution, lspace_resolution,
	                  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	e_light::unif_view = render_ctx.proj *
			render_ctx.view * render_ctx.model;
	e_light::unif_pos = l->pos;
	e_light::unif_rgb = l->rgb;
	e_light::unif_cascade_bounds = bounds;

	return true;
}

static void lighting_pass ()
{
	current_sspace_fbo ^= 1;
	sspace_fbo[current_sspace_fbo].apply();

	glUseProgram(lighting_program);

	bind_tex2d_to_slot(0, sspace_fbo[current_sspace_fbo ^ 1].color[0]->id);
	bind_tex2d_to_slot(1, lspace_fbo.color[0]->id);
	bind_tex2d_to_slot(2, gbuf_fbo.color[GBUF_SLOT_WORLD_POS]->id);
	bind_tex2d_to_slot(3, gbuf_fbo.color[GBUF_SLOT_WORLD_NORM]->id);

	glUniform3fv(UNIFORM_LOC_LIGHT_POS, 1, value_ptr(e_light::unif_pos));
	glUniform3fv(UNIFORM_LOC_LIGHT_RGB, 1, value_ptr(e_light::unif_rgb));
	glUniformMatrix4fv(UNIFORM_LOC_LIGHT_VIEW, 1, false,
			value_ptr(e_light::unif_view));

	gbuffer_pass();
}

void compute_lighting ()
{
	sspace_fbo[0].apply();
	glClearColor(ambient.x, ambient.y, ambient.z, 1.0);
	glClear(GL_COLOR_BUFFER_BIT);
	current_sspace_fbo = 0;

	render_ctx.stage = RENDER_STAGE_LIGHTING_LSPACE;
	for (e_light* l: lights) {
		if (fill_depth_map(l))
			lighting_pass();
	}
}

vec3 ambient = vec3(0);
COMMAND_ROUTINE (light_ambience)
{
	if (ev != PRESS)
		return;
	switch (args.size()) {
	case 1:
		ambient = vec3(atof(args[0].c_str()));
		break;
	case 3:
		ambient = vec3(atof(args[0].c_str()),
		               atof(args[1].c_str()),
		               atof(args[2].c_str()));
		break;
	default:
		return;
	}
}
