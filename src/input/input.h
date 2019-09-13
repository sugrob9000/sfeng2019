#ifndef INPUT_H
#define INPUT_H

#include "inc_general.h"
#include "inc_gl.h"
#include "core/core.h"

namespace input
{

struct t_command
{
	std::string cmd;
};

void init (std::string input_conf_path);
void handle_input ();

t_command parse_command (std::string str);
void run_command (t_command& cmd);

}

#endif // INPUT_H

