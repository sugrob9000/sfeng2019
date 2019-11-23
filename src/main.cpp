#include "inc_general.h"
#include "render/render.h"
#include "core/core.h"
#include "input/input.h"
#include "input/bind.h"
#include "input/console.h"

#include <chrono>

int main (int argc, char** argv)
{
	init_input();
	init_core();
	init_render();

	console.close();

	camera = t_camera(vec3(), vec3(), 2000.0f, 1.0f, 60.0f);

	run_script("res/cfg/rc");
	run_argv_commands(argc, argv);

	while (!must_quit) {
		namespace cr = std::chrono;
		using sc = cr::steady_clock;
		sc::time_point start = sc::now();

		handle_input();
		update();
		render_all();
	}

	SDL_Quit();
	return exit_code;
}

