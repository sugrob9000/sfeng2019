#include "inc_general.h"
#include "core.h"
#include "render/render.h"

namespace core
{

t_game_info game;

void init ()
{
	game.must_quit = false;
	game.exit_code = 0;
}

void error (const std::string msg, bool fatal)
{
	std::cerr << "Error: " << msg << std::endl;
	if (fatal) {
		game.must_quit = true;
		game.exit_code = 1;
	}
}

void warn (const std::string msg)
{
	std::cerr << "Warning: " << msg << std::endl;
}

void t_entity::render ()
{
	render_info->render(pos);
}

}
