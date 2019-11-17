#include "vis.h"
#include "resource.h"
#include "core/entity.h"

void t_bound_box::render () const
{
	const vec3& s = start;
	const vec3& e = end;
	auto quad = [] (vec3 a, vec3 b, vec3 c, vec3 d)
	{
		glVertex3f(a.x, a.y, a.z);
		glVertex3f(b.x, b.y, b.z);
		glVertex3f(c.x, c.y, c.z);
		glVertex3f(d.x, d.y, d.z);
	};

	glBegin(GL_QUADS);
	quad(s, { s.x, e.y, s.z }, { s.x, e.y, e.z }, { s.x, s.y, e.z });
	quad(s, { e.x, s.y, s.z }, { e.x, e.y, s.z }, { s.x, e.y, s.z });
	quad(s, { e.x, s.y, s.z }, { e.x, s.y, e.z }, { s.x, s.y, e.z });
	quad(e, { e.x, e.y, s.z }, { e.x, s.y, s.z }, { e.x, s.y, e.z });
	quad(e, { e.x, s.y, e.z }, { s.x, s.y, e.z }, { s.x, e.y, e.z });
	quad(e, { e.x, e.y, s.z }, { s.x, e.y, s.z }, { s.x, e.y, e.z });
	glEnd();
}

unsigned int occ_shader_prog;
unsigned int occ_fbo;
unsigned int occ_fbo_texture;
unsigned int occ_fbo_renderbuffer;

constexpr int occ_fbo_size = 256;

void init_vis ()
{
	occ_shader_prog = glCreateProgram();
	glAttachShader(occ_shader_prog,
		get_shader("int/vert_identity", GL_VERTEX_SHADER));
	glAttachShader(occ_shader_prog,
		get_shader("int/frag_null", GL_FRAGMENT_SHADER));
	glLinkProgram(occ_shader_prog);

	constexpr float z = 0.0;
	occ_planes.push_back(
		{ { { -100, -100, z },
		    { -100, 100, z },
		    { 100, 100, z },
		    { 100, -100, z } } } );

	glGenFramebuffers(1, &occ_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, occ_fbo);

	glGenTextures(1, &occ_fbo_texture);
	glBindTexture(GL_TEXTURE_2D, occ_fbo_texture);

	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
			occ_fbo_size, occ_fbo_size,
			0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
			GL_TEXTURE_2D, occ_fbo_texture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

std::vector<t_occlusion_plane> occ_planes;
void draw_occlusion_planes ()
{
	glUseProgram(occ_shader_prog);
	glDisable(GL_CULL_FACE);
	glEnable(GL_DEPTH_TEST);

	for (const t_occlusion_plane p: occ_planes) {
		glBegin(GL_POLYGON);
		for (const vec3& v: p.points)
			glVertex3f(v.x, v.y, v.z);
		glEnd();
	}
	glEnable(GL_CULL_FACE);
}

std::vector<e_base*> visible_set;
void fill_visible_set ()
{
	visible_set.clear();

	glViewport(0, 0, occ_fbo_size, occ_fbo_size);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, occ_fbo);

	glClear(GL_DEPTH_BUFFER_BIT);

	draw_occlusion_planes();

	int n = ents.vec.size();
	unsigned int queries[n];
	glGenQueries(n, queries);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDepthMask(GL_FALSE);
	glDisable(GL_CULL_FACE);

	for (int i = 0; i < n; i++) {
		e_base* e = ents.vec[i];

		glBeginQuery(GL_SAMPLES_PASSED, queries[i]);
		glPushMatrix();
		rotate_gl_matrix(e->ang);
		translate_gl_matrix(e->pos);

		e->get_bbox().render();

		glPopMatrix();
		glEndQuery(GL_SAMPLES_PASSED);

		unsigned int pixels;
		glGetQueryObjectuiv(queries[i], GL_QUERY_RESULT, &pixels);

		if (pixels > 0)
			visible_set.push_back(e);
	}
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

	glDeleteQueries(n, queries);
}
