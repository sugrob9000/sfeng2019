#ifndef MODEL_H
#define MODEL_H

#include "inc_general.h"

namespace render
{

class t_model
{
	public:

	struct t_vertex
	{
		vec3 pos;
		vec3 norm;
		float u, v;
	};

	std::vector<t_vertex> verts;

	t_model (std::string objpath);

	bool load_obj (std::string path);
	bool load_rvd (std::string path);

	void render () const;
};

}

#endif // MODEL_H
