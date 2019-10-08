#ifndef MATERIAL_H
#define MATERIAL_H

#include "inc_gl.h"
#include "inc_general.h"

namespace render
{

typedef unsigned int t_texture;
typedef unsigned int t_shader;

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

t_shader compile_glsl (std::string path, GLenum shadertype);
t_texture load_texture (std::string path);

}

#endif // MATERIAL_H
