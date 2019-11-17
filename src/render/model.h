#ifndef MODEL_H
#define MODEL_H

#include "inc_general.h"
#include "render/vis.h"
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

/*
 * An in-memory representation of a model,
 * which can be used for reading from/dumping to a file
 * but not suited for actual rendering
 * Mostly used for loading models, does not live long
 */
struct t_model_mem
{
	std::vector<t_vertex> verts;
	t_bound_box bbox;

	void calc_bbox ();
	bool load_obj (std::string path);
	/*
	 * Binary file format that stores the number of vertices,
	 * then vertices themselves
	 */
	bool load_rvd (std::string path);
	void dump_rvd (std::string path) const;
};

/*
 * A representation of a model which is
 * suitable for rendering (stores the internal GL object id)
 */
struct t_model
{
	unsigned int display_list_id;
	t_bound_box bbox;

	void render () const;
	void load (const t_model_mem& verts);
};

#endif // MODEL_H
