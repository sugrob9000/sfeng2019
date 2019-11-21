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

	glDisable(GL_CULL_FACE);
	glBegin(GL_QUADS);
	quad(s, { s.x, e.y, s.z }, { s.x, e.y, e.z }, { s.x, s.y, e.z });
	quad(s, { e.x, s.y, s.z }, { e.x, e.y, s.z }, { s.x, e.y, s.z });
	quad(s, { e.x, s.y, s.z }, { e.x, s.y, e.z }, { s.x, s.y, e.z });
	quad(e, { e.x, e.y, s.z }, { e.x, s.y, s.z }, { e.x, s.y, e.z });
	quad(e, { e.x, s.y, e.z }, { s.x, s.y, e.z }, { s.x, e.y, e.z });
	quad(e, { e.x, e.y, s.z }, { s.x, e.y, s.z }, { s.x, e.y, e.z });
	glEnd();
}

bool t_bound_box::point_in (vec3 pt) const
{
	return (pt.x >= start.x) && (pt.y >= start.y) && (pt.z >= start.z)
		&& (pt.x <= end.x) && (pt.y <= end.y) && (pt.z <= end.z);
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
 * An octree is used to store the world polygons, then walked to
 *   determine the currently visible set.
 * The map can specify the leaf capacity, which will get a leaf
 *   split when exceeded, and maximum leaf depth, beyond which
 *   no leaf will ever be split.
 */

int oct_leaf_capacity = 0;
int oct_max_depth = 0;

struct oct_data
{
	/*
	 * First vertex of triangle, e.g. 0, 3, 6, ...
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

	oct_node (int lvl);
	~oct_node ();
};

oct_node root(0);
t_model_mem world_tris;

/*
 * The ID of the octet in which point is
 * if the midpoint of the bbox is origin
 */
uint8_t which_octet (vec3 origin, vec3 point)
{
	return (point.x > origin.x)
	    + ((point.y > origin.y) << 1)
	    + ((point.z > origin.z) << 2);
}

/*
 * The bbox of an octet with given ID, given the parent
 */
t_bound_box octet_bound (t_bound_box parent, uint8_t octet_id)
{
	vec3 mid = (parent.start + parent.end) * 0.5;
	t_bound_box r = parent;
	(octet_id & 1 ? r.start : r.end).x = mid.x;
	(octet_id & 2 ? r.start : r.end).y = mid.y;
	(octet_id & 4 ? r.start : r.end).z = mid.z;
	return r;
}

void oct_node::build (t_bound_box bounds)
{
	if (level > oct_max_depth)
		return;
	if (bucket.size() <= oct_leaf_capacity)
		return;

	leaf = false;
	for (int i = 0; i < 8; i++)
		children[i] = new oct_node(level + 1);

	vec3 bb_mid = (bounds.start + bounds.end) * 0.5;

	for (oct_data d: bucket) {

		vec3 tri_mid = { };
		for (int i = 0; i < 3; i++)
			tri_mid += world_tris.verts[d.triangle_index + i].pos;
		tri_mid /= 3.0;

		children[which_octet(bb_mid, tri_mid)]->bucket.push_back(d);
	}

	bucket.clear();

	for (int i = 0; i < 8; i++)
		children[i]->build(octet_bound(bounds, i));
}

oct_node::oct_node (int lvl)
{
	leaf = true;
	level = lvl;
}

oct_node::~oct_node ()
{
	if (leaf)
		return;
	for (int i = 0; i < 8; i++)
		delete children[i];
}

void draw_octree_sub (oct_node* node, t_bound_box b)
{
	glUseProgram(0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glColor3f(0.5, 0.5, 0.5);
	b.render();
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glBegin(GL_TRIANGLES);
	vec3 color = (0.5 * (b.start + b.end)) / 1000.0;
	glColor4f(color.x, color.y, color.z, 1.0);
	for (oct_data d: node->bucket) {
		for (int i = 0; i < 3; i++) {
			int idx = d.triangle_index + i;
			const vec3& v = world_tris.verts[idx].pos;
			glVertex3f(v.x, v.y, v.z);
		}
	}
	glEnd();

	if (!node->leaf) {
		for (int i = 0; i < 8; i++)
			draw_octree_sub(node->children[i], octet_bound(b, i));
	}
}

void draw_octree ()
{
	draw_octree_sub(&root, world_tris.bbox);
}

t_bound_box bounds_override;
void read_world_vis_data (std::string path)
{
	std::ifstream f(path);
	if (!f)
		fatal("Vis data for world unavailable: %s", path.c_str());

	std::string option;
	while (f >> option) {
		if (option == "oct_depth") {
			f >> oct_max_depth;
		} else if (option == "oct_capacity") {
			f >> oct_leaf_capacity;
		} else if (option == "bounds") {
			f >> bounds_override.start >> bounds_override.end;
		} else {
			fatal("Unrecognized option in vis data %s: %s",
					path.c_str(), option.c_str());
		}
	}
}

void read_world_geo (std::string obj_path)
{
	world_tris.load_obj(obj_path);

	if (bounds_override.volume() > 0.0)
		world_tris.bbox = bounds_override;

	for (int i = 0; i < world_tris.verts.size(); i += 3)
		root.bucket.push_back({ i });

	root.build(world_tris.bbox);
}

