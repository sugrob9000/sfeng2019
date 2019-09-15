#include "cmds_basic.h"
#include "core/core.h"

namespace input
{
namespace cmd
{

CONSOLE_COMMAND (nop) { }

CONSOLE_COMMAND (exit)
{
	core::due_to_quit = true;
}

}
}
