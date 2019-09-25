#include "inc_general.h"
#include "core.h"
#include "render/render.h"

namespace core
{

t_game_info game;

t_game_info::t_game_info ()
{
	must_quit = false;
	exit_code = 0;
	tick = 0;
}

void t_game_info::update ()
{
	tick++;
}

}
