#ifndef VIS_H
#define VIS_H

#include "inc_general.h"
#include "core/core.h"
#include "render.h"
#include "material.h"
#include "model.h"

void init_vis ();
void vis_initialize_world (std::string path);

/*
 * An octree is used to store the world polygons, then walked to
 *   determine the currently visible set.
 * The map can specify the leaf capacity, which will get a leaf
 *   split when exceeded, and maximum leaf depth, beyond which
 *   no leaf will ever be split.
 *
 * The map specifies certain "occlusion planes", which are polygons
 *   that are rendered every frame into a depth buffer and against
 *   which the leaves in the octree are tested. (These are rendered
 *   into a single display list)
 */

struct t_world_triangle
{
	t_vertex v[3];
	t_material* mat;
};

struct oct_node
{
	/* Triangles within this node with the same material */
	struct mat_group {
		t_material* mat;
		unsigned int display_list;
	};

	/*
	 * Invariant, after build() has been called:
	 * for leaves, the vector material_buckets is in a valid state;
	 * otherwise, the vector bucket is in a valid state and empty
	 */
	union {
		/* First vertex of triangle in world_tris, eg 0, 3, 6 */
		std::vector<int> bucket;
		std::vector<mat_group> material_buckets;
	};

	bool leaf;
	oct_node* children[8];
	t_bound_box actual_bounds;

	void build (t_bound_box bounds, int level);
	void make_leaf ();

	void walk_for_vis (const vec3& cam) const;
	void render_tris () const;

	oct_node ();
	~oct_node ();
};

extern std::vector<const oct_node*> visible_leaves;

void vis_render_bbox (const t_bound_box& b);
void vis_fill_visible (const vec3& cam);
void vis_debug_renders ();

#endif // VIS_H
