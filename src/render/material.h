#ifndef MATERIAL_H
#define MATERIAL_H

#include "inc_general.h"

namespace render
{

class t_texture
{
	public:

	unsigned int id;

	t_texture ();
	t_texture (std::string path);

	bool load (std::string path);
};

class t_material
{
	private:

	unsigned int frag;
	unsigned int vert;
	unsigned int program;

	public:

	t_material (std::string pathmtf);

	bool load_mtf (std::string path);
	void apply ();
};

unsigned int compile_glsl (std::string path, GLenum shadertype);

}

#endif // MATERIAL_H
