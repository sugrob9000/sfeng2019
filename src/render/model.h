#ifndef MODEL_H
#define MODEL_H

#include "material.h"
#include "core/core.h"
#include <vector>

struct t_vertex
{
	vec3 pos;
	vec3 norm;
	vec2 tex;
};
bool operator< (const t_vertex&, const t_vertex&);

/*
 * An in-memory representation of a model for loading,
 * conversion, etc. but not for actual rendering.
 */
struct t_model_mem
{
	struct vertex {
		t_vertex v;
		vec3 tangent;
	};
	struct triangle {
		int index[3];
		t_material* material;
	};

	std::vector<vertex> vertices;
	std::vector<triangle> triangles;

	t_bound_box bbox;

	void calc_bbox ();

	const t_vertex& get_vertex (int tri, int vert) const
	{ return vertices[triangles[tri].index[vert]].v; }

	void gl_send_triangle (int tri_id) const;

	void load_obj (const std::string& path);

	/*
	 * RVD (raw vertex data) - a binary file format
	 * that should be faster to load than OBJ.
	 * Format:
	 *   number of vertices
	 *   each vertex, as raw float data
	 *   number of material buckets
	 *   each material bucket, as:
	 *     length of name
	 *     name, NOT null terminated
	 *     number of vertices (should divide by 3)
	 *     triangles, as triplets of indices into vertices
	 */
	void load_rvd (const std::string& path);
	void dump_rvd (const std::string& path) const;
};

/*
 * The representation of a model which is efficient to render
 */
struct t_model
{
	GLuint display_list_id;

	t_bound_box bbox;

	void render () const;
	void load (const t_model_mem& src);
};

#endif // MODEL_H
