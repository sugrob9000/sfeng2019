#include "inc_general.h"
#include "core.h"

namespace core
{

bool due_to_quit;

void init ()
{
	due_to_quit = false;
}

void error (const std::string msg, bool fatal)
{
	std::cerr << "Error: " << msg << std::endl;
	if (fatal)
		due_to_quit = true;
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
