#ifndef INPUT_H
#define INPUT_H

#include "inc_general.h"
#include "inc_gl.h"
#include "core/core.h"
#include <sstream>

namespace input
{

typedef std::vector<std::string> t_command_args;

/*
 * Instances of this class are *not* supposed to be created
 * a lot during runtime. The key binding maps holds instances of them,
 * which are created once during engine start.
 * They are also created when parsing a console command, which is
 * not often either.
 */
class t_command
{
	public:

	std::string cmd;
	t_command_args args;

	void run () const;
};

/*
 * Instances of this class are not supposed to be created much
 * in runtime either. They are created on engine start, when the
 * commands get registered. Thus, the only thing that can shall
 * get rapidly created is a scancode structure.
 */
struct t_action
{
	void (*press) (t_command_args);
	void (*release) (t_command_args);
};

void init (std::string input_conf_path);
void handle_input ();

t_command parse_command (std::string str);

}

#endif // INPUT_H

