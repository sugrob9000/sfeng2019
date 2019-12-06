#include "vis.h"
#include "resource.h"
#include "core/core.h"
#include "input/cmds.h"
#include <algorithm>
#include <cstring>

int oct_leaf_capacity = 0;
int oct_max_depth = 0;

oct_node* root;

GLuint occ_fbo;
GLuint occ_fbo_texture;
GLuint occ_shader_prog;
GLuint occ_planes_display_list;
/* To query OpenGL as to whether the bounding box is visible */
GLuint occ_queries[8];

/*
 * Rather than being screen-sized, the occlusion framebuffer
 * is a smallish square
 */
constexpr int occ_fbo_size = 256;

std::vector<t_world_triangle> world_tris;
t_bound_box world_bounds_override;

/*
 * The ID of the octant in which point is
 * if the midpoint of the bbox is origin
 */
uint8_t which_octant (vec3 origin, vec3 point)
{
	return (point.x > origin.x)
	    + ((point.y > origin.y) << 1)
	    + ((point.z > origin.z) << 2);
}

/*
 * The bbox of an octant with given ID, given the parent
 */
t_bound_box octant_bound (t_bound_box parent, uint8_t octant_id)
{
	vec3 mid = (parent.start + parent.end) * 0.5;
	t_bound_box r = parent;
	(octant_id & 1 ? r.start : r.end).x = mid.x;
	(octant_id & 2 ? r.start : r.end).y = mid.y;
	(octant_id & 4 ? r.start : r.end).z = mid.z;
	return r;
}

void oct_node::build (t_bound_box bounds, int level)
{
	// bounds must include the triangles entirely
	for (int d: bucket) {
		for (int i = 0; i < 3; i++)
			bounds.update(world_tris[d].v[i].pos);
	}
	actual_bounds = bounds;

	if (level > oct_max_depth || bucket.size() <= oct_leaf_capacity) {
		make_leaf();
		return;
	}

	is_leaf = false;
	for (int i = 0; i < 8; i++)
		children[i] = new oct_node;

	vec3 bb_mid = (bounds.start + bounds.end) * 0.5;

	for (int d: bucket) {
		vec3 tri_mid(0.0);
		for (int i = 0; i < 3; i++)
			tri_mid += world_tris[d].v[i].pos;
		tri_mid /= 3.0;
		children[which_octant(bb_mid, tri_mid)]->bucket.push_back(d);
	}

	bucket.clear();

	for (int i = 0; i < 8; i++)
		children[i]->build(octant_bound(bounds, i), level + 1);
}

void oct_node::make_leaf ()
{
	std::map<t_material*, std::vector<t_vertex>> m;

	for (const int& d: bucket) {
		for (int i = 0; i < 3; i++) {
			m[world_tris[d].mat].push_back(
				world_tris[d].v[i]);
		}
	}

	// the bucket is in union with leaf data
	// so we have to destroy and construct them manually
	bucket.~vector();
	new (&leaf) leaf_data;

	leaf.mat_buckets.reserve(m.size());

	for (std::pair<t_material* const, std::vector<t_vertex>>& p: m) {
		t_material* mat = p.first;
		std::vector<t_vertex>& vertices = p.second;

		unsigned int dlist = glGenLists(1);
		glNewList(dlist, GL_COMPILE);
		glBegin(GL_TRIANGLES);
		for (const t_vertex& v: vertices) {
			glNormal3f(v.norm.x, v.norm.y, v.norm.z);
			glTexCoord2f(v.tex.u, v.tex.v);
			glVertex3f(v.pos.x, v.pos.y, v.pos.z);
		}
		glEnd();
		glEndList();

		leaf.mat_buckets.push_back({ mat, dlist });
	}
}

void oct_node::render_tris (t_render_stage s) const
{
	for (const mat_group& gr: leaf.mat_buckets) {
		gr.mat->apply(s);
		glCallList(gr.display_list);
	}
}

oct_node::oct_node ()
{
	is_leaf = true;
	new (&bucket) std::vector<int>;
}

oct_node::~oct_node ()
{
	if (is_leaf) {
		leaf.~leaf_data();
	} else {
		bucket.~vector();
		for (int i = 0; i < 8; i++)
			delete children[i];
	}
}



std::vector<const oct_node*> visible_leaves;

void vis_render_bbox (const t_bound_box& box);
void oct_node::check_visibility (const vec3& cam) const
{
	if (is_leaf) {
		visible_leaves.push_back(this);
		return;
	}

	unsigned int child_pixels[8];

	for (int i = 0; i < 8; i++) {
		glBeginQuery(GL_SAMPLES_PASSED, occ_queries[i]);
		vis_render_bbox(children[i]->actual_bounds);
		glEndQuery(GL_SAMPLES_PASSED);
	}

	for (int i = 0; i < 8; i++) {
		glGetQueryObjectuiv(occ_queries[i],
			GL_QUERY_RESULT, &child_pixels[i]);
	}

	for (int i = 0; i < 8; i++) {
		// we always want to render the node we are in, but its
		// polygons may be culled, so pass it specially
		if (child_pixels[i] > 0
		|| children[i]->actual_bounds.point_in(cam))
			children[i]->check_visibility(cam);
	}
}

void vis_fill_visible (const vec3& cam)
{
	// setup occlusion rendering
	glBindFramebuffer(GL_FRAMEBUFFER, occ_fbo);
	glViewport(0, 0, occ_fbo_size, occ_fbo_size);
	glClear(GL_DEPTH_BUFFER_BIT);
	glUseProgram(occ_shader_prog);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	// fill z-buffer with data from occlusion planes
	glDepthMask(GL_TRUE);
	glCallList(occ_planes_display_list);
	glDepthMask(GL_FALSE);

	// walk the tree nodes which pass the z-test (and are on screen)
	visible_leaves.clear();
	root->check_visibility(cam);
}



void read_world_vis_data (std::string path)
{
	std::ifstream f(path);
	if (!f)
		warning("Vis data for world unavailable: %s", path.c_str());

	std::string option;
	while (f >> option) {
		if (option == "oct_depth") {
			f >> oct_max_depth;
		} else if (option == "oct_capacity") {
			f >> oct_leaf_capacity;
		} else if (option == "bounds") {
			f >> world_bounds_override.start >>
				world_bounds_override.end;
		} else {
			warning("Unrecognized option in vis data %s: %s",
					path.c_str(), option.c_str());
		}
	}
}

void read_world_obj (const std::string& path);
void vis_initialize_world (const std::string& path)
{
	read_world_obj(path + "/geo.obj");
	read_world_vis_data(path + "/vis");

	if (world_bounds_override.volume() <= 0.0) {
		fatal("Map %s does not specify valid world bounds",
				path.c_str());
	}

	root = new oct_node;

	for (int i = 0; i < world_tris.size(); i++)
		root->bucket.push_back({ i });

	root->build(world_bounds_override, 0);
}

/*
 * Read world in a different way than what t_model_mem does
 * because we're better off working with faces rather than vertices,
 * and we need to handle materials specified in the obj in
 * a very special way (ie OCCLUDE means it's an occlusion plane
 *                     and goes to a different vector)
 * TODO: maybe redesign OBJ reading to reduce the blatant
 *   code duplication - the only things that are different
 *   are usemtl and face handling
 */
void read_world_obj (const std::string& path)
{
	std::ifstream f(path);
	if (!f)
		fatal("world obj not found: %s", path.c_str());

	std::vector<vec3> points;
	std::vector<vec3> normals;
	std::vector<t_texcrd> texcoords;

	// we will draw them into a display list right away
	std::vector<vec3> occ_plane_verts;

	auto pack =
		[] (char a, char b) constexpr -> uint16_t
		{ return ((a << 8) | b); };

	t_material* cur_material = get_material("worldmat");
	bool occlusion_plane_mode = false;

	for (std::string line; std::getline(f, line); ) {

		int comment = line.find('#');
		if (comment != std::string::npos)
			line.erase(comment);

		if (line.size() < 2)
			continue;

		uint16_t p = pack(line[0], line[1]);

		switch (p) {
		case pack('v', ' '): {
			// vertex
			float x, y, z;
			sscanf(line.c_str(), "%*s %f %f %f", &x, &y, &z);
			points.push_back({ x, y, z });
			break;
		}
		case pack('v', 'n'): {
			// vertex normal
			float x, y, z;
			sscanf(line.c_str(), "%*s %f %f %f", &x, &y, &z);
			normals.push_back({ x, y, z });
			break;
		}
		case pack('v', 't'): {
			// tex coord
			float u, v;
			sscanf(line.c_str(), "%*s %f %f", &u, &v);
			texcoords.push_back({ u, 1.0f - v });
			break;
		}
		case pack('f', ' '): {
			// face
			int v[3];
			int n[3];
			int t[3];
			sscanf(line.c_str(), "%*s %i/%i/%i %i/%i/%i %i/%i/%i",
					&v[0], &t[0], &n[0],
					&v[1], &t[1], &n[1],
					&v[2], &t[2], &n[2]);

			if (occlusion_plane_mode) {
				for (int i = 0; i < 3; i++) {
					occ_plane_verts.push_back(
						points[v[i]-1]);
				}
			} else {
				t_world_triangle tri;
				for (int i = 0; i < 3; i++) {
					tri.v[i] = { points[v[i]-1],
					             normals[n[i]-1],
					             texcoords[t[i]-1] };
				}
				tri.mat = cur_material;
				world_tris.push_back(tri);
			}

			break;
		}
		case pack('u', 's'): {
			// usemtl - update current material
			char buf[line.length()];
			sscanf(line.c_str(), "%*s %s", buf);
			if (strncmp(buf, "OCCLUDE", 7) == 0) {
				occlusion_plane_mode = true;
			} else {
				occlusion_plane_mode = false;
				cur_material = get_material(buf);
			}
			break;
		}
		default: {
			// something in the format we are unaware of
			continue;
		}
		}
	}

	occ_planes_display_list = glGenLists(1);
	glNewList(occ_planes_display_list, GL_COMPILE);
	glBegin(GL_TRIANGLES);
	for (const vec3& v: occ_plane_verts)
		glVertex3f(v.x, v.y, v.z);
	glEnd();
	glEndList();
}


void vis_render_bbox (const t_bound_box& box)
{
	const vec3& s = box.start;
	const vec3& e = box.end;
	auto quad = [] (vec3 a, vec3 b, vec3 c, vec3 d) -> void {
		glVertex3f(a.x, a.y, a.z);
		glVertex3f(b.x, b.y, b.z);
		glVertex3f(c.x, c.y, c.z);
		glVertex3f(d.x, d.y, d.z);
	};

	glBegin(GL_QUADS);
	quad(s, { s.x, e.y, s.z }, { s.x, e.y, e.z }, { s.x, s.y, e.z });
	quad(s, { e.x, s.y, s.z }, { e.x, e.y, s.z }, { s.x, e.y, s.z });
	quad(s, { s.x, s.y, e.z }, { e.x, s.y, e.z }, { e.x, s.y, s.z });
	quad(e, { e.x, e.y, s.z }, { e.x, s.y, s.z }, { e.x, s.y, e.z });
	quad(e, { e.x, s.y, e.z }, { s.x, s.y, e.z }, { s.x, e.y, e.z });
	quad(e, { s.x, e.y, e.z }, { s.x, e.y, s.z }, { e.x, e.y, s.z });
	glEnd();
}

void init_vis ()
{
	// setup the framebuffer in which to test for occlusion
	// the shader for occlusion rendering just does nothing
	// but write to the z-buffer
	occ_shader_prog = glCreateProgram();
	glAttachShader(occ_shader_prog,
		get_shader("common/vert_identity", GL_VERTEX_SHADER));
	glAttachShader(occ_shader_prog,
		get_shader("common/frag_null", GL_FRAGMENT_SHADER));
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

	glGenQueries(8, occ_queries);
}

bool debug_draw_wireframe = false;
COMMAND_ROUTINE (vis_worldwireframe)
{
	debug_draw_wireframe = (ev == PRESS);
}

bool debug_draw_occ_planes = false;
COMMAND_ROUTINE (vis_occluders)
{
	debug_draw_occ_planes = (ev == PRESS);
}

bool debug_draw_leaves = false;
COMMAND_ROUTINE (vis_leaves)
{
	debug_draw_leaves = (ev == PRESS);
}

void vis_debug_renders ()
{
	glUseProgram(0);
	if (debug_draw_wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glColor4f(0.0, 0.0, 0.0, 0.5);
		for (const oct_node* node: visible_leaves) {
			for (const auto& gr: node->leaf.mat_buckets)
				glCallList(gr.display_list);
		}
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
	}

	if (debug_draw_occ_planes) {
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glColor4f(0.8, 0.2, 0.2, 0.5);
		glCallList(occ_planes_display_list);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
	}

	if (debug_draw_leaves) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glLineWidth(1.5);
		glColor4f(1.0, 0.0, 0.0, 0.3);

		for (const oct_node* leaf: visible_leaves)
			vis_render_bbox(leaf->actual_bounds);

		glLineWidth(1.0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
	}
}
