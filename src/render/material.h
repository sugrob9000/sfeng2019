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

	std::string name;
	std::vector<GLuint> bitmap_texture_ids;

	void load (const std::string& path);
	void apply (t_render_stage stage = SHADE_FINAL) const;
};

void init_materials ();

t_shader_id compile_glsl (std::string path, GLenum shadertype);
t_texture_id load_texture (std::string path);

int get_surface_gl_format (SDL_Surface* s);

/* Draws nothing, for various edge cases */
extern t_material* mat_none;
/* Not an actual material, used in vis for occlusion planes */
extern t_material* mat_occlude;

/*
 * Discard previous material info to assure that the next
 * material will actually be applied
 */
void material_barrier ();

/* The first N texture slots are reserved for engine use
 * (lighting buffers etc.); after Nth, the slots are used for user bitmaps */
constexpr int MAT_TEXTURE_SLOT_OFFSET = 2;

#endif // MATERIAL_H
