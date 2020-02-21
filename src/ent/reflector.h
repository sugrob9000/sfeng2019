#ifndef ENT_REFLECTOR_H
#define ENT_REFLECTOR_H

#include "misc.h"
#include "inc_gl.h"
#include "core/entity.h"
#include "render/framebuffer.h"

class e_reflector: public e_base
{
	public:

	GLuint ref_fbo;
	GLuint cubemap;
	int resolution;

	ENT_MEMBERS (reflector);

	void post_init ();
	void render_cubemap ();
};

extern std::vector<e_reflector*> reflectors;

void compute_reflections ();

#endif