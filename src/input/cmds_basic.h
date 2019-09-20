#ifndef CMDS_BASIC_H
#define CMDS_BASIC_H

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
 * Output arguments to stderr
 */
COMMAND_ROUTINE (echo);

}
}

#endif
