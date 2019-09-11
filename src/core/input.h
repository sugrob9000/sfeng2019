#ifndef INPUT_H
#define INPUT_H

#include "inc_general.h"
#include "inc_gl.h"

namespace input
{

struct t_command
{
	std::string cmd;
};

typedef std::map<SDL_Scancode, t_command> t_keybind_map;
extern t_keybind_map key_binds;

void init (std::string input_conf_path);
void handle_input ();

t_command parse_command (std::string str);
void run_command (t_command& cmd);

/*
 * Returns 0 on success, -1 on failure to open file,
 * line number on failure to parse file
 *
 * Syntax, one per line:
 * keyname:t_command
 *
 * No unnecessary whitespace is allowed, except
 * fully empty lines.
 * Comments start with # (must be the first character!).
 */
int keybinds_from_cfg (std::string path, t_keybind_map& m);

}

#endif // INPUT_H

