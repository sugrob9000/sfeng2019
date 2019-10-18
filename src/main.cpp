#include "inc_general.h"
#include "render/render.h"
#include "core/core.h"
#include "input/input.h"
#include "input/bind.h"

using core::game;

int main (int argc, char** argv)
{
	render::init(640, 480);
	input::init("res/cfg/input");
	core::init();
	render::camera = render::t_camera(vec3(), vec3(), 500.0, 1.0, 60);

	game.load_map("res/maps/map1");

	while (!core::game.must_quit) {
		input::handle_input();
		game.update();
		render::render_all();
	}

	SDL_Quit();
	return core::game.exit_code;
}

