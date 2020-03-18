#include "core/core.h"
#include "core/entity.h"
#include "input/bind.h"
#include "input/cmds.h"
#include "input/input.h"
#include "misc.h"
#include "render/ctx.h"
#include "render/render.h"
#include "render/framebuffer.h"

COMMAND_ROUTINE (sun_at_view)
{
	if (ev != PRESS)
		return;
	std::string pos_str = vec3toa(camera.pos);
	std::string ang_str = vec3toa(camera.ang);
	run_cmd_ext("signal 0 sun setpos " + pos_str);
	run_cmd_ext("signal 0 sun setang " + ang_str);
}

int main (int argc, const char* const* argv)
{
	init_input();
	init_core();
	init_render();

	DEBUG_EXPR(sizeof(t_fbo));

	console_close();

	camera = t_camera({ 605.542, 4.64112, 217.669 },
	                  { 18, 0, -45 },
	                  5000.0f, 0.5f, 60.0f, 1.0);

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

