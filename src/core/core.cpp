#include "inc_general.h"
#include "core.h"

namespace core
{

bool due_to_quit;

void init ()
{
	due_to_quit = false;
}

void t_entity::render ()
{
	render_info->render(pos);
}

}
