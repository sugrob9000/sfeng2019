#ifndef VIS_H
#define VIS_H

#include "inc_general.h"
#include "core/core.h"
#include "core/entity.h"
#include "render.h"
#include "material.h"
#include "model.h"

void init_vis ();
void vis_initialize_world (const std::string& path);

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
		GLuint display_list;
	};

	struct leaf_data {
		std::vector<mat_group> mat_buckets;
		std::vector<e_base*> entities;
	};

	union {
		/*
		 * Indices into the internal world triangle array.
		 * In non-leaves, after tree has been built,
		 *   is in a valid state and empty
		 */
		std::vector<int> bucket;

		/* In leaves, this is in a valid state */
		leaf_data leaf;
	};

	bool is_leaf;
	oct_node* children[8];
	t_bound_box actual_bounds;

	void build (t_bound_box bounds, int level);
	void make_leaf ();

	void check_visibility (const vec3& cam) const;
	void query_entity (const e_base* e);

	void render_tris (t_render_stage s = SHADE_FINAL) const;

	oct_node ();
	~oct_node ();
};

extern std::vector<const oct_node*> visible_leaves;
extern std::vector<const e_base*> visible_entities;

void vis_render_bbox (const t_bound_box& b);
void vis_fill_visible (const vec3& cam);
void vis_debug_renders ();

#endif // VIS_H
