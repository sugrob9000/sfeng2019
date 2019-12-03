#ifndef ENT_LIGHT_H
#define ENT_LIGHT_H

#include "core/entity.h"
#include "render/render.h"

class e_light: public e_base
{
	public:

	float cone_angle;
	float reach;

	SDL_Color color;

	e_light ();

	ENT_MEMBERS (light)
};

constexpr float LIGHT_Z_NEAR = 10.0;

extern GLuint lspace_fbo_texture;
extern GLuint sspace_fbo_texture;

extern std::vector<e_light*> lights;

void init_lighting ();
void compute_lighting ();

#endif // ENT_LIGHT_H
