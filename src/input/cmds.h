#ifndef CMDS_H
#define CMDS_H

#include "input.h"

#define COMMAND_ROUTINE(name) \
	void name (const t_cmd_args& args, uint8_t ev)
#define MOUSEMOVE_ROUTINE(name) \
	void name (int dx, int dy, int abx, int aby)

namespace input
{
namespace cmd
{

/*
 * Do nothing.
 */
COMMAND_ROUTINE (nop);

/*
 * Exit the engine.
 */
COMMAND_ROUTINE (exit);

/*
 * Output arguments to stdout
 */
COMMAND_ROUTINE (echo);

/*
 * Bind a key to a command.
 * The key's name is the first argument
 * (In key names that contain spaces,
 *  they are replaced with underscores)
 * The command is the rest of the arguments
 */
COMMAND_ROUTINE (bind);

MOUSEMOVE_ROUTINE (basic_mousemove);

}
}

#endif // CMDS_H
