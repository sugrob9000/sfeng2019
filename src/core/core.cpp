#include "inc_general.h"
#include "core.h"

namespace core
{

t_game_info game_info;

void init ()
{
	game_info.must_quit = false;
	game_info.exit_code = 0;
}

void error (const std::string msg, bool fatal)
{
	std::cerr << "Error: " << msg << std::endl;
	if (fatal) {
		game_info.must_quit = true;
		game_info.exit_code = 1;
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
