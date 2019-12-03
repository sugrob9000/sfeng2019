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

	camera = t_camera({ 0.0, 0.0, 200.0 }, { 0.0, 0.0, 90.0 },
			5000.0f, 0.5f, 60.0f);

	run_script("res/cfg/rc");
	run_argv_commands(argc, argv);


	e_base* light = ents.spawn("light");
	t_ent_keyvals kv;
	kv.add("pos", "20.0 0.0 200.0");
	kv.add("ang", "0.0 0.0 0.0");
	kv.add("name", "light");
	kv.add("reach", "200");
	light->apply_keyvals(kv);

	e_base* car = ents.spawn("prop");
	kv.clear();
	kv.add("pos", "20.0 140.0 180.0");
	kv.add("ang", "0.0 0.0 30.0");
	kv.add("model", "muscle_car");
	kv.add("mat", "car-red");
	car->apply_keyvals(kv);


	while (!must_quit) {
		handle_input();
		update();
		render_all();
	}

	SDL_Quit();
	return exit_code;
}

