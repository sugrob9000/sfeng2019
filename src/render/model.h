#ifndef MODEL_H
#define MODEL_H

#include "inc_general.h"

namespace render
{

class t_model
{
	public:

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

	std::vector<t_vertex> verts;

	t_model ();

	bool load_obj (std::string path);
	bool load_rvd (std::string path);
	void dump_rvd (std::string path);

	void render () const;
};

}

#endif // MODEL_H
