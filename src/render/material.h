#ifndef MATERIAL_H
#define MATERIAL_H

#include "inc_gl.h"
#include "render.h"
#include <vector>

struct t_material
{
	GLuint program;

	std::string name;
	std::vector<GLuint> bitmap_texture_ids;

	void load (const std::string& path);
	void apply () const;
};

void init_materials ();

GLuint load_texture (std::string path);

GLenum get_surface_gl_format (SDL_Surface* s);

/* Draws nothing, for various edge cases */
extern t_material* mat_none;
/* Not an actual material, used in vis for occlusion planes */
extern t_material* mat_occlude;

/*
 * Discard previous material info to assure that the next
 * material will actually be applied
 */
void material_barrier ();

/*
 * The first N texture slots are reserved for engine use
 * (lighting buffers etc.); after Nth, the slots are used for user bitmaps
 */
constexpr int MAT_TEXTURE_SLOT_OFFSET = 2;


GLuint make_glsl_program (const std::vector<GLuint>& shaders);
GLuint compile_glsl (std::string path, GLenum shadertype);

#endif // MATERIAL_H
