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


/*
 * Octets:
 *     x ->
 * low   0 1 | y
 *       2 3 v
 * high: 4 5
 *       6 7
 */
constexpr int OCTREE_LEAF_CAPACITY = 50;
constexpr int OCTREE_MAX_DEPTH = 20;

t_model_mem* p_world_tris;
struct oct_data
{
	/*
	 * index of the first vert in world_tris,
	 * e.g. 0, 3, 6, ...
	 */
	int triangle_index;
};

struct oct_node
{
	std::vector<oct_data> bucket;
	bool leaf;
	int level;
	oct_node* children[8];

	void build (t_bound_box bounds);

	oct_node (): leaf(true), level(0) { }
	~oct_node ();
};

oct_node root;
t_model_mem world_tris;

uint8_t which_octet (vec3 origin, vec3 point)
{
	return (point.x > origin.x)
	    + ((point.y > origin.y) << 1)
	    + ((point.z > origin.z) << 2);
}

t_bound_box octet_bound (t_bound_box b, int octet_num)
{
	const vec3& s = b.start;
	const vec3& e = b.end;
	vec3 m = (s + e) * 0.5;

	t_bound_box r;

	if (octet_num & 1) {
		r.start.x = s.x;
		r.end.x = m.x;
	} else {
		r.start.x = m.x;
		r.end.x = e.x;
	}
	if (octet_num & 1) {
		r.start.y = s.y;
		r.end.y = m.y;
	} else {
		r.start.y = m.y;
		r.end.y = e.y;
	}
	if (octet_num & 1) {
		r.start.z = s.z;
		r.end.z = m.z;
	} else {
		r.start.z = m.z;
		r.end.z = e.z;
	}
	return r;
}

void oct_node::build (t_bound_box bounds)
{
	if (level > OCTREE_MAX_DEPTH) {
		DEBUG_EXPR(level);
		return;
	}
	if (bucket.size() <= OCTREE_LEAF_CAPACITY) {
		DEBUG_EXPR(bucket.size());
		return;
	}

	leaf = false;
	for (int i = 0; i < 8; i++) {
		children[i] = new oct_node;
		children[i]->level = level + 1;
	}

	vec3 bb_mid = (bounds.start + bounds.end) * 0.5;

	for (oct_data d: bucket) {
		int i = d.triangle_index;
		vec3 tri_mid = (world_tris.verts[i].pos
		              + world_tris.verts[i + 1].pos
		              + world_tris.verts[i + 2].pos) / 3.0;
		children[which_octet(bb_mid, tri_mid)]->bucket.push_back(d);
	}
	bucket.clear();

	for (int i = 0; i < 8; i++)
		children[i]->build(octet_bound(bounds, i));
}

oct_node::~oct_node ()
{
	if (leaf)
		return;
	for (int i = 0; i < 8; i++)
		delete children[i];
}

void build_world_from_obj (std::string obj_path)
{
	world_tris.load_obj(obj_path);
	world_tris.bbox = { { -512, -512, -512 },
	                    { 512, 512, 512 } };

	for (int i = 0; i < world_tris.verts.size(); i += 3)
		root.bucket.push_back({ i });

	// DEBUG_EXPR(root.bucket[0].triangle_index);
	root.build(world_tris.bbox);
}

void draw_octree_sub (oct_node* node, t_bound_box b)
{
	glUseProgram(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glBegin(GL_QUADS);
	glColor3f(1.0, 0.0, 0.0);
	glVertex2f(b.start.x, b.start.y);
	glVertex2f(b.start.x, b.end.y);
	glVertex2f(b.end.x, b.end.y);
	glVertex2f(b.end.x, b.start.y);

	glEnd();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	if (!node->leaf) {
		for (int i = 0; i < 8; i++)
			draw_octree_sub(node->children[i], octet_bound(b, i));
	}
}

void draw_octree ()
{
	glPushMatrix();
	glScalef(1.0 / 520.0, -1.0 / 520.0, 1.0);

	draw_octree_sub(&root, world_tris.bbox);
	glPopMatrix();
}
