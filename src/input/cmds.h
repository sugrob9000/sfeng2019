#ifndef CMDS_H
#define CMDS_H

#include "input.h"

#define COMMAND_ROUTINE(name)                                     \
	void cmd_##name ([[maybe_unused]] const t_cmd_args& args, \
		         [[maybe_unused]] uint8_t ev)

#define MOUSEMOVE_ROUTINE(name)                      \
	void mouse_##name ([[maybe_unused]] int dx,  \
		           [[maybe_unused]] int dy,  \
		           [[maybe_unused]] int abx, \
		           [[maybe_unused]] int aby)

#define COMMAND_SET_BOOL(name, bool_to_set)  \
	COMMAND_ROUTINE (name)               \
	{                                    \
		bool_to_set = (ev == PRESS); \
	}

#define COMMAND(name) \
	COMMAND_ROUTINE (name);
#include "cmds.inc"
#undef COMMAND

MOUSEMOVE_ROUTINE (mousemove_camera);

#endif // CMDS_H
