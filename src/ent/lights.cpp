#include "ent/lights.h"
#include "input/cmds.h"
#include "render/framebuffer.h"
#include "render/render.h"
#include "render/resource.h"
#include "render/vis.h"

std::vector<e_light*> lights;

/* ============= e_light code ============= */

SIG_HANDLER (light, setcolor)
{
	atovec3(arg, ent->rgb);
}

FILL_IO_DATA (light)
{
	BASIC_SIG_HANDLERS(light);
	SET_SIG_HANDLER(light, setcolor);
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

mat4 e_light::uniform_view;
vec3 e_light::uniform_pos;
vec3 e_light::uniform_rgb;

/*
 * Depth maps from lights' perspective
 */
constexpr int lspace_samples = 4;
constexpr int lspace_resolution = 1024;

/* Render to multisampled, then blit into regular for sampling */
t_fbo lspace_fbo_ms;
t_fbo lspace_fbo;

/*
 * Screen space shadow maps
 */
t_fbo sspace_fbo[2];
int current_sspace_fbo = 0;


static void init_screenspace ()
{
	int w = sdlctx.res_x;
	int h = sdlctx.res_y;

	auto depth_rbo = make_rbo(w, h, GL_DEPTH_COMPONENT);
	for (int i: { 0, 1 }) {
		sspace_fbo[i].make()
			.attach_color(make_tex2d(w, h, GL_RGB16F))
			.attach_depth(depth_rbo)
			.assert_complete();
		sspace_add_buffer(sspace_fbo[i]);
	}
}

void init_lighting ()
{
	init_screenspace();

	// lightspace FBO
	constexpr int s = lspace_resolution;

	lspace_fbo_ms.make()
		.attach_color(make_rbo_msaa(
			s, s, GL_RGBA32F, lspace_samples))
		.attach_depth(make_rbo_msaa(
			s, s, GL_DEPTH_COMPONENT, lspace_samples))
		.assert_complete();

	lspace_fbo.make()
		.attach_color(make_tex2d(s, s, GL_RGBA32F))
		.assert_complete();
}


void light_apply_uniforms ()
{
}


void fill_depth_map (const e_light* l)
{
	lspace_fbo_ms.apply();
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	mat4 restore_proj = render_ctx.proj;
	mat4 restore_view = render_ctx.view;
	mat4 restore_model = render_ctx.model;

	l->view();
	l->vis.render();

	e_light::uniform_view = render_ctx.proj *
			render_ctx.view * render_ctx.model;
	e_light::uniform_pos = l->pos;
	e_light::uniform_rgb = l->rgb;

	render_ctx.proj = restore_proj;
	render_ctx.view = restore_view;
	render_ctx.model = restore_model;

	glBindFramebuffer(GL_READ_FRAMEBUFFER, lspace_fbo_ms.id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lspace_fbo.id);
	glBlitFramebuffer(0, 0, lspace_resolution, lspace_resolution,
	                  0, 0, lspace_resolution, lspace_resolution,
	                  GL_COLOR_BUFFER_BIT, GL_NEAREST);
}

void compute_lighting ()
{
	for (e_light* l: lights) {
		render_ctx.stage = RENDER_STAGE_LIGHTING_LSPACE;
		fill_depth_map(l);
	}
}

vec3 ambient = vec3(0.25);
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
