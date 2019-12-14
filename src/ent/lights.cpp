#include "lights.h"
#include "error.h"
#include "render/render.h"
#include "render/vis.h"
#include "render/resource.h"

std::vector<e_light*> lights;

/* ============= e_light code ============= */

FILL_IO_DATA (light) { }

e_light::e_light ()
{
	lights.push_back(this);
}

void e_light::think ()
{
}

void e_light::apply_keyvals (const t_ent_keyvals& kv)
{
	apply_basic_keyvals(kv);

	KV_TRY_GET(kv["cone"],
		cone_angle = atof(val.c_str());,
		cone_angle = 60.0; );
	KV_TRY_GET(kv["reach"],
		reach = atof(val.c_str());,
		reach = 500.0; );
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

void e_light::apply_uniforms ()
{
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, lspace_fbo_texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D,
			sspace_fbo_texture[current_sspace_fbo ^ 1]);

	glUniform1i(UNIFORM_LOC_DEPTH_MAP, 0);
	glUniform1i(UNIFORM_LOC_PREV_SHADOWMAP, 1);

	glUniformMatrix4fv(UNIFORM_LOC_LIGHT_VIEWMAT, 1, false,
			uniform_viewmat);

	glUniform3fv(UNIFORM_LOC_LIGHT_POS, 1, uniform_pos.data());

	glUniform3fv(UNIFORM_LOC_LIGHT_RGB, 1, uniform_rgb.data());
}

/*
 * Light space framebuffer
 */
GLuint lspace_fbo;
GLuint lspace_fbo_texture;
constexpr int lspace_fbo_size = 1024;

/*
 * Screen space framebuffer
 */
GLuint sspace_fbo[2];
GLuint sspace_fbo_texture[2];
int current_sspace_fbo = 0;
int sspace_fbo_width;
int sspace_fbo_height;

void init_lighting ()
{
	sspace_fbo_width = CEIL_PO2(sdlcont.res_x);
	sspace_fbo_height = CEIL_PO2(sdlcont.res_y);

	auto set_fb_tex_params = [] () -> void {
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
		glTexParameteri(GL_TEXTURE_2D,
				GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,
				GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	};

	// light space framebuffer
	glGenTextures(1, &lspace_fbo_texture);
	glBindTexture(GL_TEXTURE_2D, lspace_fbo_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
			lspace_fbo_size, lspace_fbo_size, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	set_fb_tex_params();

	glGenFramebuffers(1, &lspace_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, lspace_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			GL_TEXTURE_2D, lspace_fbo_texture, 0);

	// screen space framebuffers

	glGenFramebuffers(2, sspace_fbo);
	glGenTextures(2, sspace_fbo_texture);
	GLuint depth_rbo[2];
	glGenRenderbuffers(2, depth_rbo);

	for (int i: { 0, 1 }) {
		glBindFramebuffer(GL_FRAMEBUFFER, sspace_fbo[i]);
		glBindTexture(GL_TEXTURE_2D, sspace_fbo_texture[i]);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
				sspace_fbo_width, sspace_fbo_height,
				0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
		set_fb_tex_params();
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
				GL_TEXTURE_2D, sspace_fbo_texture[i], 0);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo[i]);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
				sspace_fbo_width, sspace_fbo_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
				GL_RENDERBUFFER, depth_rbo[i]);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void fill_depth_map (const e_light* l)
{
	glBindFramebuffer(GL_FRAMEBUFFER, lspace_fbo);
	glViewport(0, 0, lspace_fbo_size, lspace_fbo_size);
	glClear(GL_DEPTH_BUFFER_BIT);
	glDepthMask(GL_TRUE);
	glDepthFunc(GL_LESS);

	// GL_PROJECTION and GL_MODELVIEW in this function are
	// as meaningless as MTX_* - we only send one matrix

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glLoadIdentity();
	gluPerspective(l->cone_angle * 2.0, 1.0, LIGHT_Z_NEAR, l->reach);
	glRotatef(-90.0, 1.0, 0.0, 0.0);
	rotate_gl_matrix(l->ang);
	translate_gl_matrix(-l->pos);

	for (const e_base* e: ents.vec)
		e->render(LIGHTING_LSPACE);
	for (const oct_node* node: visible_leaves)
		node->render_tris(LIGHTING_LSPACE);

	glGetFloatv(GL_MODELVIEW_MATRIX, e_light::uniform_viewmat);
	e_light::uniform_rgb = l->rgb;
	e_light::uniform_pos = l->pos;

	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void compose_add_depth_map ()
{
	current_sspace_fbo ^= 1;

	glBindFramebuffer(GL_FRAMEBUFFER, sspace_fbo[current_sspace_fbo]);
	glViewport(0, 0, sspace_fbo_width, sspace_fbo_height);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	for (const oct_node* node: visible_leaves)
		node->render_tris(LIGHTING_SSPACE);
	draw_visible_entities(LIGHTING_SSPACE);
}

void compute_lighting ()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	for (int i: { 0, 1 }) {
		glBindFramebuffer(GL_FRAMEBUFFER, sspace_fbo[i]);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	for (const e_light* l: lights) {
		fill_depth_map(l);
		compose_add_depth_map();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, sdlcont.res_x, sdlcont.res_y);
}
