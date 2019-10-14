#ifndef CMDS_H
#define CMDS_H

#include "input.h"

#define COMMAND_ROUTINE(name)                               \
	void name ([[maybe_unused]] const t_cmd_args& args, \
		   [[maybe_unused]] uint8_t ev)

#define MOUSEMOVE_ROUTINE(name)              \
	void name ([[maybe_unused]] int dx,  \
		   [[maybe_unused]] int dy,  \
		   [[maybe_unused]] int abx, \
		   [[maybe_unused]] int aby)

namespace input
{
namespace cmd
{

#define COMMAND(name) \
	COMMAND_ROUTINE (name);
#include "cmds.inc"
#undef COMMAND

MOUSEMOVE_ROUTINE (mousemove_camera);

}
}

#endif // CMDS_H
