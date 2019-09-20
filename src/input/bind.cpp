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

SDL_Scancode bind_get_from_name (std::string name)
{
	for (char& c: name)
		c = tolower(c);

	if (name == "lmb")
		return mouse_scancodes[SDL_BUTTON_LEFT];
	else if (name == "mmb")
		return mouse_scancodes[SDL_BUTTON_MIDDLE];
	else if (name == "rmb")
		return mouse_scancodes[SDL_BUTTON_RIGHT];

	return SDL_GetScancodeFromName(name.c_str());
}

int t_keybind_map::load_from_cfg (std::string path)
{
	std::ifstream f(path);

	if (!f.good())
		return -1;

	clear();

	int line_nr = 1;

	for (std::string line; std::getline(f, line); line_nr++) {

		int comment = line.find('#');
		if (comment != std::string::npos)
			line.erase(comment, std::string::npos);

		if (line.empty())
			continue;

		int colon = line.find(':');
		if (colon == std::string::npos)
			return line_nr;

		std::string keyname = line.substr(0, colon);
		std::string bind = line.substr(colon + 1, std::string::npos);

		SDL_Scancode scan = bind_get_from_name(keyname);
		if (scan == SDL_SCANCODE_UNKNOWN)
			return line_nr;

		add_bind(scan, parse_command(bind));
	}

	return 0;
}

}
