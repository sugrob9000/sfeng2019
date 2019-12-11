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

t_bound_box world_bounds_override;
t_model_mem world;

std::vector<const oct_node*> visible_leaves;

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
	for (int i = 0; i < 3; i++)
		(octant_id & (1 << i) ? r.start : r.end)[i] = mid[i];
	return r;
}

void oct_node::build (t_bound_box bounds, int level)
{
	// bounds must include the triangles entirely
	for (int d: bucket) {
		for (int i = 0; i < 3; i++)
			bounds.update(world.get_vertex(d, i).pos);
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
			tri_mid += world.get_vertex(d, i).pos;
		tri_mid /= 3.0;
		children[which_octant(bb_mid, tri_mid)]->bucket.push_back(d);
	}

	bucket.clear();

	for (int i = 0; i < 8; i++)
		children[i]->build(octant_bound(bounds, i), level + 1);
}

void oct_node::make_leaf ()
{
	std::map<t_material*, std::vector<int>> m;

	for (const int& d: bucket)
		m[world.triangles[d].material].push_back(d);

	// the bucket is in union with leaf data
	// so we have to destroy and construct them manually

	mat_buckets.reserve(m.size());

	for (const auto& p: m) {
		t_material* mat = p.first;
		const std::vector<int>& tri_ids = p.second;

		unsigned int dlist = glGenLists(1);
		glNewList(dlist, GL_COMPILE);
		glBegin(GL_TRIANGLES);
		for (int i: tri_ids)
			world.gl_send_triangle(i);
		glEnd();
		glEndList();

		mat_buckets.push_back({ mat, dlist });
	}
}

void oct_node::render_tris (t_render_stage s) const
{
	for (const mat_group& gr: mat_buckets) {
		gr.mat->apply(s);
		glCallList(gr.display_list);
	}
}

oct_node::oct_node ()
{
	is_leaf = true;
}

oct_node::~oct_node ()
{
	if (!is_leaf) {
		for (int i = 0; i < 8; i++)
			delete children[i];
	}
}


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
	glDisable(GL_CULL_FACE);
	glDepthFunc(GL_LESS);

	// fill z-buffer with data from occlusion planes
	glDepthMask(GL_TRUE);
	glCallList(occ_planes_display_list);
	glDepthMask(GL_FALSE);

	// walk the tree nodes which pass the z-test (and are on screen)
	visible_leaves.clear();
	root->check_visibility(cam);
}



void oct_node::requery_entity (e_base* e, const t_bound_box& b)
{
	auto p = std::find(entities_inside.begin(), entities_inside.end(), e);
	uint8_t before = (p != entities_inside.end());
	uint8_t now = b.intersects(actual_bounds);

	switch (now | (before << 1)) {
	case 0b00:
		// was not in before, did not enter.
		// absolutely nothing to do
		return;
	case 0b01:
		// entered
		entities_inside.push_back(e);
		break;
	case 0b10:
		// exited
		*p = entities_inside.back();
		entities_inside.pop_back();
		break;
	case 0b11:
		// was in before and did not exit,
		// but might have exited or entered a child
		break;
	}
	if (!is_leaf) {
		for (int i = 0; i < 8; i++)
			children[i]->requery_entity(e, b);
	}
}

void vis_requery_entity (e_base* e)
{
	t_bound_box b = e->get_bbox();
	root->requery_entity(e, e->get_bbox());
}

/*
 * When we walk the entities in the leaves like this, there
 *   is redundancy (multiple leaves that we see will touch
 *   the same entity), so we need to ensure each entity is
 *   drawn once.
 * So, define a guard key that is unique for each invocation
 *   of the function and use it to determine if we have already
 *   seen any particular entity in this invocation
 */
void draw_visible_entities (t_render_stage s)
{
	static uint64_t guard_key = 0;
	guard_key++;

	for (const oct_node* l: visible_leaves) {
		for (e_base* e: l->entities_inside) {
			if (e->render_last_guard_key == guard_key)
				continue;
			e->render_last_guard_key = guard_key;
			e->render(s);
		}
	}
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

void vis_initialize_world (const std::string& path)
{
	read_world_vis_data(path + "/vis");

	world.load_obj(path + "/geo.obj");

	if (world_bounds_override.volume() > 0.0)
		world.bbox = world_bounds_override;

	root = new oct_node;

	// Prepare the occlusion planes right away
	occ_planes_display_list = glGenLists(1);
	glNewList(occ_planes_display_list, GL_COMPILE);
	glBegin(GL_TRIANGLES);
	int n = world.triangles.size();

	for (int i = 0; i < n; i++) {
		const t_triangle& tri = world.triangles[i];
		if (tri.material == mat_occlude) {
			for (int j = 0; j < 3; j++) {
				const vec3& ps = world.get_vertex(i, j).pos;
				glVertex3f(ps.x, ps.y, ps.z);
			}
		} else {
			root->bucket.push_back(i);
		}
	}
	glEnd();
	glEndList();

	root->build(world.bbox, 0);
}



void vis_render_bbox (const t_bound_box& box)
{
	const vec3& s = box.start;
	const vec3& e = box.end;

	auto quad = [] (vec3 a, vec3 b, vec3 c, vec3 d)
	-> void {
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
bool debug_draw_occ_planes = false;
bool debug_draw_leaves = false;
bool debug_draw_leaves_nodepth = true;
COMMAND_SET_BOOL(vis_worldwireframe, debug_draw_wireframe);
COMMAND_SET_BOOL(vis_occluders, debug_draw_occ_planes);
COMMAND_SET_BOOL(vis_leaves, debug_draw_leaves);
COMMAND_SET_BOOL(vis_leaves_nodepth, debug_draw_leaves_nodepth);

void vis_debug_renders ()
{
	glUseProgram(0);

	if (debug_draw_wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glColor4f(0.0, 0.0, 0.0, 0.5);
		for (const oct_node* node: visible_leaves) {
			for (const auto& gr: node->mat_buckets)
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

		if (debug_draw_leaves_nodepth)
			glDisable(GL_DEPTH_TEST);

		glDisable(GL_CULL_FACE);
		glLineWidth(1.5);
		for (const oct_node* leaf: visible_leaves) {
			float r = leaf->entities_inside.empty() ? 0.0 : 1.0;
			glColor4f(r, 0.0, 0.0, 0.3);
			vis_render_bbox(leaf->actual_bounds);
		}

		glLineWidth(1.0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
	}
}
