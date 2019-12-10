#ifndef MODEL_H
#define MODEL_H

#include "inc_general.h"
#include "inc_gl.h"
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

/*
 * An in-memory representation of a model
 */
struct t_model_mem
{
	struct vert_internal {
		t_vertex v;
		vec3 tangent;
		vec3 bitangent;
	};
	std::vector<vert_internal> vertices;

	std::vector<int> indices;

	t_bound_box bbox;

	void calc_bbox ();
	void load_obj (std::string path);

	void load_rvd (std::string path);
	void dump_rvd (std::string path) const;
};

/*
 * A representation of a model which is suitable for rendering
 */
struct t_model
{
	GLuint display_list_id;

	t_bound_box bbox;

	void render () const;
	void load (const t_model_mem& verts);
};

void send_triangles (const std::vector<t_vertex> verts);

#endif // MODEL_H
