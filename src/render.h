#ifndef RENDER_H
#define RENDER_H

#include "inc_gl.h"

namespace render
{

class t_render_info
{
	public:

	void render (vec3 pos);
};

extern SDL_Window* window;
extern SDL_GLContext context;

bool init (int resx, int resy);

}

#endif // RENDER_H

