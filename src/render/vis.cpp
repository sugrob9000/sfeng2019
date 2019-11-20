#include "vis.h"
#include "resource.h"
#include "core/entity.h"
#include <algorithm>

float t_bound_box::volume () const
{
	return (end.x - start.x)
	     * (end.y - start.y)
	     * (end.z - start.z);
}

t_bound_box t_bound_box::updated (const t_bound_box& other) const
{
	return { { std::min(start.x, other.start.x),
	           std::min(start.y, other.start.y),
	           std::min(start.z, other.start.z) },
	         { std::max(end.x, other.end.x),
	           std::max(end.y, other.end.y),
	           std::max(end.z, other.end.z) } };
}

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

void build_world_from_obj (std::string obj_path)
{
	t_model_mem world_tris;
	world_tris.load_obj(obj_path);

	// TODO
}

