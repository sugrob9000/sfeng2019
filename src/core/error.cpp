#include "error.h"
#include "inc_general.h"
#include "core.h"

namespace core
{

void error (std::string msg, bool fatal)
{
	std::cerr << "Error: " << msg << std::endl;
	if (fatal) {
		game.must_quit = true;
		game.exit_code = 1;
	}
}

void warn (std::string msg)
{
	std::cerr << "Warning: " << msg << std::endl;
}

}
