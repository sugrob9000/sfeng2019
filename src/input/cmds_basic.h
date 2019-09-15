#ifndef CMDS_BASIC_H
#define CMDS_BASIC_H

#include "input.h"

#define COMMAND_ROUTINE(name) void name (const t_cmd_args& args, uint8_t ev)

namespace input
{
namespace cmd
{

/*
 * Do nothing. A command may need it, for instance,
 * if it does nothing on release
 */
COMMAND_ROUTINE (nop);

/*
 * Exit the engine.
 */
COMMAND_ROUTINE (exit);

}
}

#endif
