#include "lights.h"
#include "error.h"
#include "render/render.h"
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

/* ======================================== */

float e_light::uniform_viewmat[16];
vec3 e_light::uniform_rgb;
vec3 e_light::uniform_pos;

/* Depth maps from lights' perspective */
t_fbo lspace_fbo;
constexpr int lspace_resolution = 1024;

/* Screen space lighting */
t_fbo sspace_fbo[2];
int current_sspace_fbo = 0;

void init_lighting ()
{
	for (int i: { 0, 1 }) {
		sspace_fbo[i].make(CEIL_PO2(sdlcont.res_x),
		                   CEIL_PO2(sdlcont.res_y),
		                   t_fbo::BIT_DEPTH | t_fbo::BIT_COLOR);
	}

	lspace_fbo.make(lspace_resolution, lspace_resolution,
			t_fbo::BIT_DEPTH);

	// make custom color target texture for VSM
	// with two 32-bit float channels

	GLuint lspace_color;
	glGenTextures(1, &lspace_color);
	glBindTexture(GL_TEXTURE_2D, lspace_color);

	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F,
			lspace_resolution, lspace_resolution, 0,
			GL_RGBA, GL_FLOAT, nullptr);

	lspace_fbo.attach_color(lspace_color);
}

void e_light::view () const
{
	gluPerspective(cone_angle * 2.0, 1.0, LIGHT_Z_NEAR, reach);
	glRotatef(-90.0, 1.0, 0.0, 0.0);
	rotate_gl_matrix(ang);
	translate_gl_matrix(-pos);
}

void fill_depth_map (const e_light* l)
{
	lspace_fbo.apply();

	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);
	glDisable(GL_BLEND);
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

	for (const e_light* l: lights) {
		fill_depth_map(l);
		compose_add_depth_map();
	}
}
