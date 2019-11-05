#ifndef CMDS_H
#define CMDS_H

#include "input.h"

#define _COMMAND_ROUTINE_H(name)                            \
	void name ([[maybe_unused]] const t_cmd_args& args, \
		   [[maybe_unused]] uint8_t ev)

#define _MOUSEMOVE_ROUTINE_H(name)           \
	void name ([[maybe_unused]] int dx,  \
		   [[maybe_unused]] int dy,  \
		   [[maybe_unused]] int abx, \
		   [[maybe_unused]] int aby)

/*
 * The macros above get around the requirement that the
 * routines be declared within actual namespace { }'s.
 *
 * The ones below should be used.
 */

#define COMMAND_ROUTINE(name) _COMMAND_ROUTINE_H(::input::cmd::name)
#define MOUSEMOVE_ROUTINE(name) _MOUSEMOVE_ROUTINE_H(::input::cmd::name)

namespace input
{
namespace cmd
{

#define COMMAND(name) \
	_COMMAND_ROUTINE_H (name);
#include "cmds.inc"
#undef COMMAND

_MOUSEMOVE_ROUTINE_H (mousemove_camera);

}
}

#endif // CMDS_H
