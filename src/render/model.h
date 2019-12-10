#ifndef MODEL_H
#define MODEL_H

#include "inc_general.h"
#include "material.h"
#include "core/core.h"
#include <vector>

struct t_texcrd
{
	float u;
	float v;
};

struct t_vertex
{
	vec3 pos;
	vec3 norm;
	t_texcrd tex;
};

/* Trivially go through each member, nothing meaningful */
bool operator< (const t_texcrd& a, const t_texcrd& b);
bool operator< (const t_vertex& a, const t_vertex& b);

struct t_triangle
{
	int index[3];
	t_material* material;
};

/*
 * An in-memory representation of a model for loading,
 * conversion, etc. but not for actual rendering.
 */
struct t_model_mem
{
	struct vert_internal {
		t_vertex v;
		vec3 tangent;
		vec3 bitangent;
	};
	std::vector<vert_internal> vertices;
	std::vector<t_triangle> triangles;

	t_bound_box bbox;

	void calc_bbox ();
	void load_obj (std::string path);

	const t_vertex& get_vertex (int tri, int vert) const
	{ return vertices[triangles[tri].index[vert]].v; }

	void gl_send_triangle (int tri_id) const;
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
