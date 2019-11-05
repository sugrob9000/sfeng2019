#include "cmds.h"
#include "core/core.h"

COMMAND_ROUTINE (nop)
{
	return;
}

COMMAND_ROUTINE (exit)
{
	if (ev != PRESS)
		return;
	core::exit_code = 0;
	if (!args.empty())
		core::exit_code = std::atoi(args[0].c_str());
	core::must_quit = true;
}

COMMAND_ROUTINE (echo)
{
	if (ev != PRESS)
		return;

	for (const std::string& arg: args)
		std::cout << arg << " ";

	std::cout << std::endl;
}
