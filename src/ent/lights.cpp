#include "lights.h"
#include "error.h"
#include "render/render.h"
#include "render/vis.h"
#include "render/shaderlib.h"
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

float e_light::uniform_viewmat[16];
vec3 e_light::uniform_rgb;
vec3 e_light::uniform_pos;

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


void init_sspace ();
void init_lspace ();
void init_lspace_ms ();
void init_lighting ()
{
	init_sspace();
	init_lspace();
	init_lspace_ms();
}

void init_sspace ()
{
	for (int i: { 0, 1 }) {
		sspace_fbo[i].make(CEIL_PO2(sdlcont.res_x),
		                   CEIL_PO2(sdlcont.res_y),
		                   t_fbo::BIT_DEPTH | t_fbo::BIT_COLOR);
	}
}

void lspace_tex_params ()
{
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void init_lspace ()
{
	// no need for even a depth buffer
	// because we will just blit into this
	lspace_fbo.make(lspace_resolution, lspace_resolution, 0);

	// make custom color target texture for EVSM
	// with two 32-bit float channels
	GLuint lspace_color;
	glGenTextures(1, &lspace_color);
	glBindTexture(GL_TEXTURE_2D, lspace_color);
	lspace_tex_params();

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
			lspace_resolution, lspace_resolution, 0,
			GL_RGBA, GL_FLOAT, nullptr);

	lspace_fbo.attach_color(lspace_color);
}

void init_lspace_ms ()
{
	// make another lightspace framebuffer, with MSAA
	// into which we will actually render
	lspace_fbo_ms.make(lspace_resolution, lspace_resolution, 0);

	glGenTextures(1, &lspace_fbo_ms.tex_color);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, lspace_fbo_ms.tex_color);

	lspace_tex_params();

	glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, lspace_samples,
		GL_RGBA32F, lspace_resolution, lspace_resolution, GL_TRUE);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
		GL_TEXTURE_2D_MULTISAMPLE, lspace_fbo_ms.tex_color, 0);

	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, lspace_samples,
		GL_DEPTH_COMPONENT, lspace_resolution, lspace_resolution);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER, rbo);
}



COMMAND_ROUTINE (light_refit_buffers)
{
	if (ev != PRESS)
		return;
	for (int i: { 0, 1 }) {
		glDeleteFramebuffers(1, &sspace_fbo[i].id);
		const GLuint tex[2] = { sspace_fbo[i].tex_depth,
		                        sspace_fbo[i].tex_color };
		glDeleteTextures(2, tex);
		sspace_fbo[i].make(CEIL_PO2(sdlcont.res_x),
		                   CEIL_PO2(sdlcont.res_y),
				   t_fbo::BIT_DEPTH | t_fbo::BIT_COLOR);
	}
}

void fill_depth_map (const e_light* l)
{
	lspace_fbo_ms.apply();

	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	push_reset_matrices();
	glMatrixMode(GL_MODELVIEW);

	l->view();
	l->vis.render(LIGHTING_LSPACE);

	glGetFloatv(GL_MODELVIEW_MATRIX, e_light::uniform_viewmat);
	e_light::uniform_pos = l->pos;
	e_light::uniform_rgb = l->rgb;

	pop_matrices();

	glBindFramebuffer(GL_READ_FRAMEBUFFER, lspace_fbo_ms.id);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, lspace_fbo.id);
	glBlitFramebuffer(0, 0, lspace_resolution, lspace_resolution,
	                  0, 0, lspace_resolution, lspace_resolution,
	                  GL_COLOR_BUFFER_BIT, GL_NEAREST);
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

void light_apply_uniforms (t_render_stage s)
{
	if (s == LIGHTING_LSPACE)
		return;

	int shadowmap;

	if (s == LIGHTING_SSPACE) {
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, lspace_fbo.tex_color);
		glUniform1i(UNIFORM_LOC_DEPTH_MAP, 1);

		const float* pos = e_light::uniform_pos.data();
		const float* rgb = e_light::uniform_rgb.data();
		const float* mat = e_light::uniform_viewmat;
		glUniformMatrix4fv(UNIFORM_LOC_LIGHT_VIEWMAT, 1, false, mat);
		glUniform3fv(UNIFORM_LOC_LIGHT_POS, 1, pos);
		glUniform3fv(UNIFORM_LOC_LIGHT_RGB, 1, rgb);

		shadowmap = sspace_fbo[current_sspace_fbo ^ 1].tex_color;
	} else {
		shadowmap = sspace_fbo[current_sspace_fbo].tex_color;
	}

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, shadowmap);
	glUniform1i(UNIFORM_LOC_PREV_SHADOWMAP, 0);
}

void compute_lighting ()
{
	glBindFramebuffer(GL_FRAMEBUFFER, sspace_fbo[0].id);
	glClearColor(ambient.x, ambient.y, ambient.z, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	current_sspace_fbo = 0;

	glDisable(GL_BLEND);

	for (const e_light* l: lights) {
		fill_depth_map(l);
		compose_add_depth_map();
	}
}
