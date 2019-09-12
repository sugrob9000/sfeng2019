#include "inc_general.h"
#include "render.h"
#include "core/core.h"
#include "core/input.h"

render::t_render_info ri;
vec3 v = { 0.0, 0.0, 0.0 };

void render_all ()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	ri.render(v);

	SDL_GL_SwapWindow(render::window);
}

int main (int argc, char** argv)
{
	render::init(640, 480);
	input::init("res/cfg/input");

	SDL_Event ev;
	while (true) {
		input::handle_input();
		render_all();
	}

	return 0;
}

