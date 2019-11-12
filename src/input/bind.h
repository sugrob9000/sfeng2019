#ifndef BIND_H
#define BIND_H

#include "inc_general.h"
#include "input.h"

/*
 * Use obscure but valid scancodes for mouse.
 * Starts with an invalid scancode because mouse buttons
 * start from 1
 */
const SDL_Scancode scan_mouse[6] = {
		SDL_SCANCODE_UNKNOWN,
		SDL_SCANCODE_F20,
		SDL_SCANCODE_F21,
		SDL_SCANCODE_F22,
		SDL_SCANCODE_UNKNOWN,
		SDL_SCANCODE_UNKNOWN };
const SDL_Scancode scan_mwheel_up = SDL_SCANCODE_F23;
const SDL_Scancode scan_mwheel_down = SDL_SCANCODE_F24;

/*
 * A wrapper around a standard map that prevents it from
 * allocating a node when accessing a key that
 * does not exist, by instead returning a reference
 * to a static default-initialized empty command.
 */
struct t_keybind_map
{
	static t_command empty_cmd;
	std::unordered_map<SDL_Scancode, t_command> m;

	void clear ();
	void add_bind (SDL_Scancode key, t_command value);
	const t_command& operator[] (SDL_Scancode scan);
};
extern t_keybind_map key_binds;

SDL_Scancode scancode_from_name (std::string name);

#endif // BIND_H
