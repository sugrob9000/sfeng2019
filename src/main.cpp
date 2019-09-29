#include "inc_general.h"
#include "render/render.h"
#include "core/core.h"
#include "input/input.h"
#include "input/bind.h"

using core::game;

void render_all ()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

int main (int argc, char** argv)
{
	render::init(640, 480);
	input::init("res/cfg/input");
	core::init();

	game.ents.spawn("prop");

	while (!core::game.must_quit) {
		input::handle_input();
		game.update();
		render_all();
	}

	return core::game.exit_code;
}

