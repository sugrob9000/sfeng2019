#ifndef INPUT_H
#define INPUT_H

#include "inc_general.h"
#include "inc_gl.h"
#include "core/core.h"
#include <sstream>

namespace input
{

const uint8_t PRESS = 0;
const uint8_t RELEASE = 1;

typedef std::vector<std::string> t_cmd_args;

typedef void (*t_cmd_routine) (const t_cmd_args&, uint8_t);
typedef void (*t_mousemove_routine) (int dx, int dy, int abx, int aby);

/*
 * Instances of this class are *not* supposed to be created
 * a lot during runtime. The key binding maps holds instances of them,
 * which are created once during engine start.
 * They are also created when parsing a console command, which is
 * not often either.
 */
struct t_command
{
	std::string name;
	t_cmd_args args;
};

/*
 * Instances of this class are not supposed to be created much
 * in runtime either. They are created on engine start, when the
 * commands get registered. Thus, the only thing that can shall
 * get rapidly created is a scancode structure.
 */
struct t_action
{
	t_cmd_routine routine;
};

struct t_command_registry
{
	private:

	std::unordered_map<std::string, t_action> m;

	public:

	void register_command (
			std::string name,
			t_cmd_routine routine);
	void run (const t_command& cmd, uint8_t ev);
};
extern t_command_registry cmd_registry;
extern t_mousemove_routine mousemove_proc;

void init (std::string input_conf_path);
void handle_input ();

void run_script (std::string path);

t_command parse_command (std::string str);

}

#endif // INPUT_H

