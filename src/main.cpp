#include "inc_general.h"
#include "render/render.h"
#include "core/core.h"
#include "input/input.h"
#include "input/bind.h"

#include <chrono>

using core::game;

int main (int argc, char** argv)
{
	input::init();

	core::init("res/cfg/rc");
	input::run_argv_commands(argc, argv);

	render::init();
	render::camera = render::t_camera(vec3(), vec3(), 500.0, 1.0, 60);

	game.load_map("res/maps/map1");

	while (!game.must_quit) {
		namespace cr = std::chrono;
		using sc = cr::steady_clock;
		sc::time_point start = sc::now();

		input::handle_input();
		game.update();
		render::render_all();

		using frame60 = cr::duration<float, std::ratio<1, 60>>;
		while (frame60(sc::now() - start).count() < 1.0);
	}

	SDL_Quit();
	return game.exit_code;
}

