#include "core/core.h"
#include "core/entity.h"
#include "input/bind.h"
#include "input/cmds.h"
#include "input/input.h"
#include "misc.h"
#include "render/ctx.h"
#include "render/render.h"

COMMAND_ROUTINE (sun_at_view)
{
	if (ev != PRESS)
		return;

	run_cmd_ext("signal 0 sun setpos " + vec3toa(camera.pos));
	run_cmd_ext("signal 0 sun setang " + vec3toa(camera.ang));
}

int main (int argc, const char* const* argv)
{
	init_input();
	init_core();
	init_render();

	console_close();

	camera = t_camera({ 726.066, -74.7267, 214.754 },
	                  { 0, 0, -64 },
	                  2000.0f, 1.5f, 60.0f, 1.0);

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

