#include "lights.h"
#include "error.h"
#include "render/vis.h"
#include "render/resource.h"

std::vector<e_light*> lights;


FILL_IO_DATA (light) { }

e_light::e_light ()
{
	lights.push_back(this);
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
		reach = 500.0; );
}

void e_light::render () const { }
void e_light::cast_shadow () const { }
void e_light::receive_light () const { }

t_bound_box e_light::get_bbox () const { return { }; }

/*
 * Light space framebuffer
 */
GLuint lspace_fbo;
GLuint lspace_fbo_texture;
GLuint lspace_program;
constexpr int lspace_fbo_size = 1024;

/*
 * Screen space framebuffer
 */
GLuint sspace_fbo;
GLuint sspace_fbo_texture;
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

	// TODO: this should actually use the user vertex shader
	lspace_program = glCreateProgram();
	glAttachShader(lspace_program, get_shader("common/frag_null",
				GL_FRAGMENT_SHADER));
	glAttachShader(lspace_program, get_shader("common/vert_identity",
				GL_VERTEX_SHADER));
	glLinkProgram(lspace_program);

	// screen space framebuffer

	glGenFramebuffers(1, &sspace_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, sspace_fbo);

	glGenTextures(1, &sspace_fbo_texture);
	glBindTexture(GL_TEXTURE_2D, sspace_fbo_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
			sspace_fbo_width, sspace_fbo_height,
			0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	set_fb_tex_params();
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_2D, sspace_fbo_texture, 0);

	GLuint depth_rbo;
	glGenRenderbuffers(1, &depth_rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT,
			sspace_fbo_width, sspace_fbo_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			GL_RENDERBUFFER, depth_rbo);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

float lspace_view_matrix[16];

void fill_depth_map (const e_light* l)
{
	glBindFramebuffer(GL_FRAMEBUFFER, lspace_fbo);
	glViewport(0, 0, lspace_fbo_size, lspace_fbo_size);
	glClear(GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();

	glLoadIdentity();
	gluPerspective(l->cone_angle * 2.0, 1.0, LIGHT_Z_NEAR, l->reach);
	glRotatef(-90.0, 1.0, 0.0, 0.0);
	rotate_gl_matrix(l->ang);
	translate_gl_matrix(-l->pos);

	glUseProgram(lspace_program);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	for (const e_base* e: ents.vec) {
		glLoadIdentity();
		e->cast_shadow();
	}
	glPopMatrix();

	glGetFloatv(GL_MODELVIEW_MATRIX, lspace_view_matrix);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
}

void compose_add_depth_map ()
{
	glBindFramebuffer(GL_FRAMEBUFFER, sspace_fbo);
	glViewport(0, 0, sspace_fbo_width, sspace_fbo_height);

	glEnable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	for (const e_base* e: ents.vec) {
		glLoadIdentity();
		e->receive_light();
	}
	glPopMatrix();
}

void compute_lighting ()
{
	for (const e_light* l: lights) {
		fill_depth_map(l);
		compose_add_depth_map();
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, sdlcont.res_x, sdlcont.res_y);
}
