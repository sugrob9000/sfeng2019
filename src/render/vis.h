#ifndef VIS_H
#define VIS_H

#include "inc_general.h"
#include "core/core.h"
#include "core/entity.h"
#include "render.h"
#include "material.h"
#include "model.h"
#include <set>

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

struct oct_node
{
	/*
	 * Triangles within this node with the same material.
	 * The vector is empty in non-leaves!
	 */
	struct mat_group {
		t_material* mat;
		GLuint display_list;
	};
	std::vector<mat_group> mat_buckets;

	/*
	 * Indices into the triangle array of internal world model.
	 * Empty after tree has been built!
	 */
	std::vector<int> bucket;

	bool is_leaf;
	oct_node* children[8];
	t_bound_box actual_bounds;

	void build (t_bound_box bounds, int level);
	void make_leaf ();

	void check_visibility (const vec3& cam) const;
	void render_tris (t_render_stage s = SHADE_FINAL) const;

	std::vector<e_base*> entities_inside;
	void requery_entity (e_base* e, const t_bound_box& b);

	oct_node ();
	~oct_node ();
};

extern std::vector<const oct_node*> visible_leaves;

void vis_fill_visible (const vec3& cam);
void vis_requery_entity (e_base* e);
void draw_visible_entities (t_render_stage s);

void vis_debug_renders ();

#endif // VIS_H
