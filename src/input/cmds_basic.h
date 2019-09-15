#ifndef CMDS_BASIC_H
#define CMDS_BASIC_H

#include "input.h"

#define CONSOLE_COMMAND(name) void name (const t_cmd_args& args)

namespace input
{
namespace cmd
{

/*
 * Do nothing. A command may need it, for instance,
 * if it does nothing on release
 */
CONSOLE_COMMAND (nop);

/*
 * Exit the engine.
 */
CONSOLE_COMMAND (exit);

}
}

#endif
