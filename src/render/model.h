#ifndef MODEL_H
#define MODEL_H

#include "inc_general.h"

namespace render
{

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


class t_vertices
{
	public:

	std::vector<t_vertex> verts;

	bool load_obj (std::string path);

	/*
	 * RVD (raw vertex data) is a file format that stores
	 * the model's data in the RAM-efficient format:
	 * - store the number of vertices
	 * - store the raw contents of verts.data()
	 */
	bool load_rvd (std::string path);
	void dump_rvd (std::string path);
};

class t_model
{
	public:

	unsigned int display_list_id;

	void render () const;
	void load (const t_vertices& verts);
};

}

#endif // MODEL_H
