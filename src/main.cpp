#include "inc_general.h"
#include "render/render.h"
#include "core/core.h"
#include "core/entity.h"
#include "input/input.h"
#include "input/bind.h"
#include "input/console.h"

int main (int argc, char** argv)
{
	init_input();
	init_core();
	init_render();

	console_close();

	camera = t_camera({ 0.0, 0.0, 200.0 }, { 0.0, 0.0, 0.0 },
			5000.0f, 0.5f, 60.0f);

	run_script("res/cfg/rc");
	run_argv_commands(argc, argv);

	while (!must_quit) {
		handle_input();
		update();
		render_all();
	}

	SDL_Quit();
	return exit_code;
}

