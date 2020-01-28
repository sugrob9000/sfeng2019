#ifndef INPUT_H
#define INPUT_H

#include "misc.h"
#include "inc_gl.h"
#include "core/core.h"
#include <sstream>
#include <unordered_map>

const uint8_t PRESS = 0;
const uint8_t RELEASE = 1;

typedef std::vector<std::string> t_cmd_args;

typedef void (*f_cmd_routine) (const t_cmd_args&, uint8_t);
typedef void (*f_mousemove_routine) (int dx, int dy, int abx, int aby);

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

struct t_command_registry
{
	std::unordered_map<std::string, f_cmd_routine> m;

	void register_command (
			std::string name,
			f_cmd_routine routine);
	void run (const t_command& cmd, uint8_t ev);
};
extern t_command_registry cmd_registry;
extern f_mousemove_routine mousemove_proc;

void init_input ();
void run_argv_commands (int argc, const char* const* argv);
void handle_input ();

/*
 * Supports prepending + or - for PRESS and RELEASE, respectively
 * (+ is unnecessary, since PRESS is the default anyway)
 */
void run_cmd_ext (const std::string& cmd);
void run_script (std::string path);

t_command parse_command (const char* cmd);


/*
 * A console window for entering commands
 * Enter - execute
 * Esc - get out
 * Shift-backspace - erase current command
 */
void console_handle_input_ev (const SDL_Event& e);
void console_open ();
void console_close ();
void console_render ();
extern bool console_active;

#endif // INPUT_H
