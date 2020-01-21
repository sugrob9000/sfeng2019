#include "lights.h"
#include "error.h"
#include "render/render.h"
#include "render/framebuffer.h"
#include "render/vis.h"
#include "render/resource.h"
#include "input/cmds.h"

std::vector<e_light*> lights;

/* ============= e_light code ============= */

FILL_IO_DATA (light)
{
	BASIC_SIG_HANDLERS(light);
}

e_light::e_light ()
{
	lights.push_back(this);
}

void e_light::moved ()
{
	e_base::moved();

	// update visible set
	reset_matrices();
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

void e_light::render (t_render_stage s) const { }

t_bound_box e_light::get_bbox () const { return { }; }

void e_light::view () const
{
	gluPerspective(cone_angle * 2.0, 1.0, LIGHT_Z_NEAR, reach);
	glRotatef(-90.0, 1.0, 0.0, 0.0);
	rotate_gl_matrix(ang);
	translate_gl_matrix(-pos);
}



/* ======================================== */

float e_light::uniform_view[16];
vec3 e_light::uniform_pos;
vec3 e_light::uniform_rgb;

/*
 * Depth maps from lights' perspective
 */
constexpr int lspace_samples = 4;
constexpr int lspace_resolution = 1024;

/* Multisampled framebuffer in which to do the rendering */
t_fbo lspace_fbo_ms;

/* Regular framebuffer into which to blit from the _ms one */
t_fbo lspace_fbo;

/*
 * Screen space shadow maps
 */
t_fbo sspace_fbo[2];
int current_sspace_fbo = 0;


void init_lighting ()
{
	// screenspace FBO
	int w = CEIL_PO2(sdlcont.res_x);
	int h = CEIL_PO2(sdlcont.res_y);

	auto depth_rbo = make_rbo(w, h, GL_DEPTH_COMPONENT);
	for (int i: { 0, 1 }) {
		sspace_fbo[i].make()
			.attach_color(make_tex2d(w, h, GL_RGB))
			.attach_depth(depth_rbo)
			.assert_complete();
	}

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

COMMAND_ROUTINE (light_refit_buffers)
{
	if (ev != PRESS)
		return;

	int w = CEIL_PO2(sdlcont.res_x);
	int h = CEIL_PO2(sdlcont.res_y);

	glDeleteRenderbuffers(1, &sspace_fbo[0].depth);
	auto depth_rbo = make_rbo(w, h, GL_DEPTH_COMPONENT);

	for (int i: { 0, 1 }) {
		sspace_fbo[i].width = 0;
		sspace_fbo[i].height = 0;

		glDeleteTextures(1, &sspace_fbo[i].color[0]);

		sspace_fbo[i]
			.attach_color(make_tex2d(w, h, GL_RGB))
			.attach_depth(depth_rbo)
			.assert_complete();
	}
}


void light_apply_uniforms (t_render_stage s)
{
	if (s == LIGHTING_LSPACE)
		return;

	int shadowmap;

	if (s == LIGHTING_SSPACE) {

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, lspace_fbo.color[0]);
		glUniform1i(UNIFORM_LOC_DEPTH_MAP, 1);

		const float* pos = e_light::uniform_pos.data();
		const float* rgb = e_light::uniform_rgb.data();
		const float* view = e_light::uniform_view;

		glUniform3fv(UNIFORM_LOC_LIGHT_POS, 1, pos);
		glUniform3fv(UNIFORM_LOC_LIGHT_RGB, 1, rgb);
		glUniformMatrix4fv(UNIFORM_LOC_LIGHT_VIEW, 1, false, view);

		const float* eye = camera.pos.data();
		glUniform3fv(UNIFORM_LOC_EYE_POSITION, 1, eye);

		shadowmap = sspace_fbo[current_sspace_fbo ^ 1].color[0];
	} else {
		shadowmap = sspace_fbo[current_sspace_fbo].color[0];
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowmap);
	glUniform1i(UNIFORM_LOC_PREV_SHADOWMAP, 0);
}


void fill_depth_map (const e_light* l)
{
	lspace_fbo_ms.apply();
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);

	glClearColor(1.0, 1.0, 1.0, 1.0);

	push_reset_matrices();

	glMatrixMode(GL_MODELVIEW);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	l->view();
	l->vis.render(LIGHTING_LSPACE);

	glGetFloatv(GL_MODELVIEW_MATRIX, e_light::uniform_view);
	e_light::uniform_pos = l->pos;
	e_light::uniform_rgb = l->rgb;

	// make the result of MSAA rendering available
	// for 3D sampling
	glBindFramebuffer(GL_READ_FRAMEBUFFER, lspace_fbo_ms.id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lspace_fbo.id);
	glBlitFramebuffer(0, 0, lspace_resolution, lspace_resolution,
	                  0, 0, lspace_resolution, lspace_resolution,
	                  GL_COLOR_BUFFER_BIT, GL_NEAREST);

	pop_matrices();
}

void compose_add_depth_map ()
{
	current_sspace_fbo ^= 1;

	sspace_fbo[current_sspace_fbo].apply();

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);

	glClearColor(ambient.x, ambient.y, ambient.z, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glMatrixMode(MTX_MODEL);

	visible_set.render(LIGHTING_SSPACE);
}

void compute_lighting ()
{
	sspace_fbo[0].apply();
	glClearColor(ambient.x, ambient.y, ambient.z, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	current_sspace_fbo = 0;

	glDisable(GL_BLEND);

	for (const e_light* l: lights) {
		fill_depth_map(l);
		compose_add_depth_map();
	}
}

vec3 ambient = { 0.15, 0.15, 0.15 };
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
