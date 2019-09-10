#ifndef INPUT_H
#define INPUT_H

#include "inc_general.h"
#include "inc_gl.h"

namespace input
{

struct command
{
	std::string cmd;
	std::vector<std::string> args;
};

typedef std::map<SDL_Scancode, command> keybind_map;
extern keybind_map key_bindings;

void init (std::string input_conf_path);
void handle_input ();

command parse_command (std::string str);
void run_command (command& cmd);

/*
 * Returns 0 on success, -1 on failure to open file,
 * line number on failure to parse file
 * 
 * Syntax, one per line:
 * keyname:command
 *
 * No unnecessary whitespace is allowed, except
 * fully empty lines.
 * Comments start with # (must be the first character!).
 */
int keybinds_from_cfg(std::string path, keybind_map& m);

}

#endif // INPUT_H

