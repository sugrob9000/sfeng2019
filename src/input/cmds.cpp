#include "cmds.h"
#include "core/core.h"
#include "bind.h"

namespace input
{
namespace cmd
{

COMMAND_ROUTINE (nop)
{
	return;
}

COMMAND_ROUTINE (exit)
{
	core::game.exit_code = 0;
	if (!args.empty())
		core::game.exit_code = std::atoi(args[0].c_str());

	core::game.must_quit = true;
}

COMMAND_ROUTINE (echo)
{
	if (ev != PRESS)
		return;

	for (const std::string& arg: args)
		std::cout << arg << " ";

	std::cout << std::endl;
}

COMMAND_ROUTINE (bind)
{
	if (ev != PRESS)
		return;
	if (args.size() < 2)
		return;

	std::string keyname = args[0];
	for (char& c: keyname) {
		if (c == '_')
			c = ' ';
	}

	std::string bind;
	for (int i = 1; i < args.size(); i++) {
		bind += args[i];
		bind += ' ';
	}

	key_binds.add_bind(scancode_from_name(keyname),
			parse_command(bind));
}

MOUSEMOVE_ROUTINE (basic_mousemove)
{
}

}
}
