#include "core/core.h"
#include "input/cmds.h"
#include "render/framebuffer.h"
#include "render/material.h"
#include "render/resource.h"
#include "render/vis.h"
#include <algorithm>
#include <cstring>

int oct_leaf_capacity = 0;
int oct_max_depth = 0;

oct_node* root = nullptr;
std::vector<const oct_node*> all_leaves;

/* Occlusion rendering for walking the tree */
t_fbo occ_fbo;
/* Is screenspace, but does not need full resolution */
constexpr int occ_fbo_size = 256;


static GLuint occ_planes_prog;
static GLuint occ_planes_dlist;
static GLuint occ_cube_prog;


/* Effectively disable visibility checking */
bool pass_all_nodes = false;
COMMAND_SET_BOOL (vis_disable, pass_all_nodes);

void init_vis ()
{
	occ_planes_prog = make_glsl_program(
			{ get_vert_shader("lib/vis_plane"),
			  get_frag_shader("common/null") });
	occ_cube_prog = make_glsl_program(
			{ get_vert_shader("lib/vis_cuboid"),
			  get_frag_shader("common/null") });
	occ_fbo.make()
		.attach_depth(make_rbo(
			occ_fbo_size, occ_fbo_size, GL_DEPTH_COMPONENT))
		.assert_complete();
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
	vec3 mid = (parent.start + parent.end) * 0.5f;
	t_bound_box r = parent;
	(octant_id & 1 ? r.start : r.end).x = mid.x;
	(octant_id & 2 ? r.start : r.end).y = mid.y;
	(octant_id & 4 ? r.start : r.end).z = mid.z;
	return r;
}

void oct_node::build (t_bound_box b, int level)
{
	// bounds must include the triangles entirely
	for (int d: bucket) {
		for (int i = 0; i < 3; i++)
			b.update(world.get_vertex(d, i).pos);
	}
	bounds = b;

	if (level >= oct_max_depth || bucket.size() <= oct_leaf_capacity) {
		make_leaf();
		return;
	}

	children = new oct_node[8];

	vec3 bb_mid = (bounds.start + bounds.end) * 0.5f;

	for (int d: bucket) {
		vec3 tri_mid(0.0);
		for (int i = 0; i < 3; i++)
			tri_mid += world.get_vertex(d, i).pos;
		tri_mid /= 3.0;
		children[which_octant(bb_mid, tri_mid)].bucket.push_back(d);
	}

	vector_clear_dealloc(bucket);

	for (int i = 0; i < 8; i++)
		children[i].build(octant_bound(bounds, i), level + 1);
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
	children = nullptr;
	glGenQueries(1, &query);
}

oct_node::~oct_node ()
{
	if (children)
		delete[] children;
	glDeleteQueries(1, &query);
}


void t_visible_set::fill (const vec3& cam)
{
	if (pass_all_nodes) {
		leaves = all_leaves;
		return;
	}
	leaves.clear();

	occ_fbo.apply();
	glClear(GL_DEPTH_BUFFER_BIT);

	// draw occlusion planes
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);
	glDisable(GL_CULL_FACE);

	glUseProgram(occ_planes_prog);
	render_ctx.submit_viewproj();
	glCallList(occ_planes_dlist);

	// attempt to draw the octree's cuboids
	glUseProgram(occ_cube_prog);
	render_ctx.submit_viewproj();

	glDepthMask(GL_FALSE);

	// BFS, but exploit the fact that a tree is bipartite,
	// parts being the even and odd depths
	std::vector<oct_node*> queues[2] = { { root }, { } };
	int cur_queue = 0;

	while (!queues[cur_queue].empty()) {
		queues[cur_queue ^ 1].clear();

		for (oct_node* n: queues[cur_queue]) {
			if (!n->children) {
				leaves.push_back(n);
				continue;
			}
			for (int i = 0; i < 8; i++) {
				glBeginQuery(GL_SAMPLES_PASSED,
						n->children[i].query);

				glUniform3fv(UNIFORM_LOC_VIS_CUBE, 2,
					n->children[i].bounds.data());
				glCallList(cuboid_dlist_outwards);

				glEndQuery(GL_SAMPLES_PASSED);
			}
		}

		for (oct_node* n: queues[cur_queue]) {
			oct_node* c = n->children;
			if (!c)
				continue;
			for (int j = 0; j < 8; j++) {
				unsigned int pixels;
				glGetQueryObjectuiv(c[j].query,
						GL_QUERY_RESULT, &pixels);
				if (pixels > 0 || c[j].bounds.point_in(cam))
					queues[cur_queue ^ 1].push_back(&c[j]);
			}
		}

		cur_queue ^= 1;
	}

	glDepthMask(GL_TRUE);
	glEnable(GL_CULL_FACE);
}



void oct_node::requery_entity (e_base* e, const t_bound_box& b)
{
	auto p = std::find(entities_inside.begin(), entities_inside.end(), e);
	uint8_t before = (p != entities_inside.end());
	uint8_t now = b.intersects(bounds);

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
	if (children) {
		for (int i = 0; i < 8; i++)
			children[i].requery_entity(e, b);
	}
}

void vis_requery_entity (e_base* e)
{
	root->requery_entity(e, e->get_bbox());
}


void t_visible_set::render () const
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
			e->render();
		}
		// draw world
		for (const auto& gr: l->mat_buckets) {
			gr.mat->apply();
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
	occ_planes_dlist = glGenLists(1);
	glNewList(occ_planes_dlist, GL_COMPILE);
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

void vis_destroy_world ()
{
	if (root != nullptr) {
		delete root;
		root = nullptr;
	}
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
	// TODO
}
