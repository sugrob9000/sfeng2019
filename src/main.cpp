#include "inc_general.h"
#include "render/render.h"
#include "core/core.h"
#include "input/input.h"
#include "input/bind.h"

core::t_entity ent;

void render_all ()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	ent.render();

	SDL_GL_SwapWindow(render::window);
}

int main (int argc, char** argv)
{
	render::init(640, 480);
	input::init("res/cfg/input");
	core::init();

	while (!core::game_info.must_quit) {
		input::handle_input();
		render_all();
	}

	return core::game_info.exit_code;
}

