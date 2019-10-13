#ifndef MATERIAL_H
#define MATERIAL_H

#include "inc_gl.h"
#include "inc_general.h"

namespace render
{

typedef unsigned int t_texture_id;
typedef unsigned int t_shader_id;

struct t_material
{
	unsigned int program;

	struct bitmap_desc {
		int location;
		t_texture_id texid;
	};

	std::vector<bitmap_desc> bitmaps;

	/*
	 * A material is described by a text file,
	 * with the following format:
	 *
	 * FRAG myfrag
	 * VERT myvert
	 * diffuse bricks.tga
	 * normal bricks-normal.tga
	 *
	 * etc.
	 * FRAG and VERT specify the names of the shaders,
	 * which resolve to res/shaders/myfrag.glsl
	 * and res/shaders/myvert.glsl, respectively.
	 * Other lines specify the names of the bitmap (texture),
	 * which resolve to res/textures/bricks.tga, etc.
	 * Materials may have their own maps defined to be used by shaders.
	 */
	void load(std::string path);
	void apply ();
};

t_shader_id compile_glsl (std::string path, GLenum shadertype);
t_texture_id load_texture (std::string path);

}

#endif // MATERIAL_H
