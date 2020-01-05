#include "inc_general.h"
#include "render/render.h"
#include "core/core.h"
#include "core/entity.h"
#include "input/input.h"
#include "input/bind.h"
#include "input/console.h"
#include "input/cmds.h"

COMMAND_ROUTINE (sun_at_view)
{
	if (ev != PRESS)
		return;
	std::string pos_str = camera.pos.to_string();
	std::string ang_str = camera.ang.to_string();
	run_cmd_ext("signal 0 sun setpos " + pos_str);
	run_cmd_ext("signal 0 sun setang " + ang_str);
}

int main (int argc, const char* const* argv)
{
	init_input();
	init_core();
	init_render();

	console_close();

	camera = t_camera({ 537.984, 72.1997, 188.145 },
	                  { 18, 0, -45 },
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

