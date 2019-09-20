#ifndef KEYBIND_H
#define KEYBIND_H

#include "inc_general.h"
#include "input.h"

namespace input
{

/*
 * Use obscure but valid scancodes for mouse.
 * Starts with an invalid scancode because mouse buttons
 * start from 1
 */
const SDL_Scancode mouse_scancodes[4] = {
		SDL_SCANCODE_UNKNOWN,
		SDL_SCANCODE_F20,
		SDL_SCANCODE_F21,
		SDL_SCANCODE_F22 };

/*
 * A wrapper around a standard map that prevents it from
 * allocating a node when accessing a key that
 * does not exist, by instead returning a reference
 * to a static default-initialized empty command.
 */
class t_keybind_map
{
	private:

	static t_command empty_cmd;
	std::unordered_map<SDL_Scancode, t_command> m;

	public:

	void clear ();
	void add_bind (SDL_Scancode key, t_command value);
	const t_command& operator[] (SDL_Scancode scan);

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
	int load_from_cfg (std::string path);
};

extern t_keybind_map key_binds;

}

#endif // KEYBIND_H
