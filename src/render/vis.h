#ifndef VIS_H
#define VIS_H

#include "core/core.h"
#include "core/entity.h"
#include "render.h"
#include "material.h"
#include "model.h"
#include <set>

void init_vis ();

void vis_initialize_world (const std::string& path);
void vis_destroy_world ();

/*
 * An octree is used to store the world polygons, then walked to
 *   determine the currently visible set.
 *
 * The map can specify the leaf capacity, which will get a leaf
 *   split when exceeded, and maximum leaf depth, beyond which
 *   no leaf will ever be split.
 *
 * The map specifies certain "occlusion planes", which are polygons
 *   that are rendered every frame into a depth buffer and against
 *   which the nodes of the octree are tested.
 */

struct t_visible_set;
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

	GLuint query;

	oct_node* children;
	t_bound_box bounds;

	void build (t_bound_box bounds, int level);
	void make_leaf ();

	std::vector<e_base*> entities_inside;
	void requery_entity (e_base* e, const t_bound_box& b);

	oct_node ();
	~oct_node ();
};

struct t_visible_set
{
	std::vector<const oct_node*> leaves;

	void fill ();
	void render () const;
	void render_debug () const;
};

extern t_visible_set all_leaves;

void vis_requery_entity (e_base* e);

#endif // VIS_H
