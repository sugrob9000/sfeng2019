#include "cmds_basic.h"
#include "core/core.h"

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
	core::due_to_quit = true;
}

}
}
