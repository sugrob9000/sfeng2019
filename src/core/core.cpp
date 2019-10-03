#include "inc_general.h"
#include "core.h"
#include "render/render.h"
#include "ent/entity.h"

namespace core
{

t_game game;

void init ()
{
}

t_game::t_game ()
{
	must_quit = false;
	exit_code = 0;
	tick = 0;
}

void t_game::update ()
{
	tick++;

	for (e_base* e: ents.v)
		e->think();
}

bool t_game::load_map (std::string path)
{
	std::ifstream f(path);

	if (!f)
		fatal("Could not open world file %s", path.c_str());

	return true;
}

}
