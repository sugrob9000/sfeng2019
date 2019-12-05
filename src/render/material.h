#ifndef MATERIAL_H
#define MATERIAL_H

#include "inc_gl.h"
#include "inc_general.h"
#include "render.h"
#include <vector>

typedef GLuint t_texture_id;
typedef GLuint t_shader_id;

struct t_material
{
	GLuint program;
	GLuint frag;
	GLuint vert;

	/*
	 * Tell the shader which stage of rendering we are at
	 */
	int render_stage_loc;

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
	 *
	 * FRAG specifies the fragment shader (resolves to res/mat/myfrag.glsl)
	 * VERT specifies the vertex shader
	 *
	 * Other lines specify the names of the bitmap (texture),
	 *   which resolve to res/mat/bricks.tga, etc.
	 * Materials may have their own maps defined to be used by shaders.
	 * The bitmap will be connected with the appropriate
	 *   uniform sampler2D in the fragment shaders:
	 *   diffuse -> uniform sampler2D map_diffuse; etc.
	 */
	void load (std::string path);
	void apply (t_render_stage stage = SHADE_FINAL) const;
};

t_shader_id compile_glsl (std::string path, GLenum shadertype);
t_texture_id load_texture (std::string path);

int get_surface_gl_format (SDL_Surface* s);

/*
 * Material that draws nothing, for various edge cases
 */
extern t_material mat_none;

/*
 * Discard previous material info to assure that the next
 * material will actually be applied
 */
void material_barrier ();

void init_materials ();

#endif // MATERIAL_H
