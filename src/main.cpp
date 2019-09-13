#include "inc_general.h"
#include "render.h"
#include "core/core.h"
#include "input/input.h"
#include "input/keybind.h"

render::t_render_info ri;
vec3 v(0.0, 0.0, 0.0);

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

	while (!core::due_to_quit) {
		input::handle_input();
		render_all();
	}

	return 0;
}

