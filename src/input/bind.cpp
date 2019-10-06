#include "bind.h"

namespace input
{

t_command t_keybind_map::empty_cmd;
t_keybind_map key_binds;

void t_keybind_map::clear ()
{
	m.clear();
}

void t_keybind_map::add_bind (SDL_Scancode scan, t_command value)
{
	m[scan] = value;
}

const t_command& t_keybind_map::operator[] (SDL_Scancode scan)
{
	auto i = m.find(scan);

	if (i == m.end())
		return empty_cmd;

	return i->second;
}

SDL_Scancode scancode_from_name (std::string name)
{
	for (char& c: name)
		c = tolower(c);

	if (name == "lmb")
		return scan_mouse[SDL_BUTTON_LEFT];
	else if (name == "mmb")
		return scan_mouse[SDL_BUTTON_MIDDLE];
	else if (name == "rmb")
		return scan_mouse[SDL_BUTTON_RIGHT];
	else if (name == "mwheelup")
		return scan_mwheel_up;
	else if (name == "mwheeldown")
		return scan_mwheel_down;

	return SDL_GetScancodeFromName(name.c_str());
}

}
