#ifndef CMDS_H
#define CMDS_H

#include "input.h"

#define COMMAND_ROUTINE(name) void name (const t_cmd_args& args, uint8_t ev)

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

}
}

#endif // CMDS_H
