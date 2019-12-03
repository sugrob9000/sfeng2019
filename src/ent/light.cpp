#include "light.h"
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

t_bound_box e_light::get_bbox () const
{
	return { };
}


GLuint light_fbo;
GLuint light_fbo_texture;
GLuint light_program;
constexpr int light_fbo_size = 1024;

void init_lighting ()
{
	glGenTextures(1, &light_fbo_texture);
	glBindTexture(GL_TEXTURE_2D, light_fbo_texture);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
			light_fbo_size, light_fbo_size, 0,
			GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &light_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			GL_TEXTURE_2D, light_fbo_texture, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	light_program = glCreateProgram();
	glAttachShader(light_program, get_shader("int/frag_null",
				GL_FRAGMENT_SHADER));
	glAttachShader(light_program, get_shader("int/vert_identity",
				GL_VERTEX_SHADER));
	glLinkProgram(light_program);
}

void compute_lighting ()
{
	glBindFramebuffer(GL_FRAMEBUFFER, light_fbo);
	glViewport(0, 0, light_fbo_size, light_fbo_size);
	
	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	glUseProgram(light_program);
	for (const e_light* l: lights) {
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(l->cone_angle * 2.0, 1.0,
				LIGHT_Z_NEAR, l->reach);
		glRotatef(90.0, 1.0, 0.0, 0.0);

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		rotate_gl_matrix(l->ang);
		translate_gl_matrix(-l->pos);

		// TODO: DOES NOT FILL PROPERLY
		for (const e_base* e: ents.vec) {
			glPushMatrix();
			translate_gl_matrix(e->pos);
			rotate_gl_matrix(e->ang);
			e->cast_shadow();
			glPopMatrix();
		}
	}

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, sdlcont.res_x, sdlcont.res_y);
}
