#ifndef KEYBIND_H
#define KEYBIND_H

#include "inc_general.h"
#include "input.h"

namespace input
{

class t_keybind_map
{
	private:

	static t_command empty_cmd;
	std::map<SDL_Scancode, t_command> m;

	public:

	void clear ();
	void add_bind (SDL_Scancode key, t_command value);
	const t_command& operator[] (SDL_Scancode scan);
};

extern t_keybind_map key_binds;

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

#endif // KEYBIND_H
