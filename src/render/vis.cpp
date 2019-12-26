#include "vis.h"
#include "resource.h"
#include "core/core.h"
#include "input/cmds.h"
#include <algorithm>
#include <cstring>

int oct_leaf_capacity = 0;
int oct_max_depth = 0;

oct_node* root;
std::vector<const oct_node*> all_leaves;

/* Occlusion rendering for walking the tree */
t_fbo occ_fbo;
/* Is screenspace, but does not need full resolution */
constexpr int occ_fbo_size = 256;

GLuint occ_shader_prog;
GLuint occ_planes_display_list;

/* To query OpenGL as to whether the bounding box is visible */
GLuint occ_queries[8];

/* Effectively disable visibility checking */
bool pass_all_nodes;
COMMAND_SET_BOOL (vis_disable, pass_all_nodes);

void init_vis ()
{
	// setup the framebuffer in which to test for occlusion
	// the shader for occlusion rendering just does nothing
	// but write to the z-buffer
	occ_shader_prog = glCreateProgram();
	glAttachShader(occ_shader_prog, get_vert_shader("common/occlude"));
	glAttachShader(occ_shader_prog, get_frag_shader("common/null"));
	glLinkProgram(occ_shader_prog);

	glGenQueries(8, occ_queries);

	occ_fbo.make(occ_fbo_size, occ_fbo_size, t_fbo::BIT_DEPTH);
}


t_bound_box world_bounds_override;
t_model_mem world;

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

	if (level >= oct_max_depth || bucket.size() <= oct_leaf_capacity) {
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
	all_leaves.push_back(this);

	std::map<t_material*, std::vector<int>> m;
	for (int d: bucket)
		m[world.triangles[d].material].push_back(d);

	mat_buckets.reserve(m.size());

	for (const auto& [mat, tri_ids]: m) {
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


void oct_node::check_visibility (const vec3& cam, t_visible_set& s) const
{
	if (is_leaf) {
		s.leaves.push_back(this);
		return;
	}

	if (pass_all_nodes) {
		for (int i = 0; i < 8; i++)
			children[i]->check_visibility(cam, s);
		return;
	}

	unsigned int child_pixels[8];

	for (int i = 0; i < 8; i++) {
		glBeginQuery(GL_SAMPLES_PASSED, occ_queries[i]);
		draw_cuboid(children[i]->actual_bounds);
		glEndQuery(GL_SAMPLES_PASSED);
	}

	for (int i = 0; i < 8; i++) {
		glGetQueryObjectuiv(occ_queries[i],
			GL_QUERY_RESULT, &child_pixels[i]);
	}

	for (int i = 0; i < 8; i++) {
		// we always want to render the node we are in,
		// but its quads may be culled, so pass it specially
		if (child_pixels[i] > 0
		|| children[i]->actual_bounds.point_in(cam))
			children[i]->check_visibility(cam, s);
	}
}

void t_visible_set::fill (const vec3& cam)
{
	if (pass_all_nodes) {
		leaves = all_leaves;
		return;
	}

	// setup occlusion rendering
	occ_fbo.apply();
	glUseProgram(occ_shader_prog);
	glEnable(GL_DEPTH_TEST);
	glClear(GL_DEPTH_BUFFER_BIT);

	// fill z-buffer with data from occlusion planes
	glDepthFunc(GL_LESS);
	glDisable(GL_CULL_FACE);
	glCallList(occ_planes_display_list);

	// walk the tree nodes which pass the z-test (and are on screen)
	glDepthMask(GL_FALSE);
	leaves.clear();
	root->check_visibility(cam, *this);

	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
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
	root->requery_entity(e, e->get_bbox());
}


void t_visible_set::render (t_render_stage s) const
{
	/*
	 * When we walk the entities in the leaves like this, there
	 *   is redundancy (multiple leaves that we see will touch
	 *   the same entity), so we need to ensure each entity is
	 *   drawn once.
	 * So, define a guard key that is unique for each invocation
	 *   and use it to determine if we have already
	 *   seen any particular entity in this invocation
	 */
	static uint64_t guard_key = 0;
	guard_key++;

	for (const oct_node* l: leaves) {
		// draw entities
		for (e_base* e: l->entities_inside) {
			if (e->render_last_guard_key == guard_key)
				continue;
			e->render_last_guard_key = guard_key;
			e->render(s);
		}
		// draw world
		for (const auto& gr: l->mat_buckets) {
			gr.mat->apply(s);
			glCallList(gr.display_list);
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
		const auto& tri = world.triangles[i];
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

bool debug_draw_wireframe = false;
bool debug_draw_occ_planes = false;
bool debug_draw_leaves = false;
bool debug_draw_leaves_nodepth = true;
COMMAND_SET_BOOL (vis_worldwireframe, debug_draw_wireframe);
COMMAND_SET_BOOL (vis_occluders, debug_draw_occ_planes);
COMMAND_SET_BOOL (vis_leaves, debug_draw_leaves);
COMMAND_SET_BOOL (vis_leaves_nodepth, debug_draw_leaves_nodepth);

void t_visible_set::render_debug () const
{
	glUseProgram(0);

	if (debug_draw_wireframe) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);
		glColor4f(0.0, 0.0, 0.0, 0.5);
		for (const oct_node* node: leaves) {
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
		glColor4f(0.0, 0.0, 0.0, 0.3);
		for (const oct_node* leaf: leaves)
			draw_cuboid(leaf->actual_bounds);
		glLineWidth(1.0);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
	}
}
