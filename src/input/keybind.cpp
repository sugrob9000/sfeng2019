#include "keybind.h"

namespace input
{

t_command t_keybind_map::empty_cmd;
t_keybind_map key_binds;

void t_keybind_map::clear ()
{
	m.clear();
}

t_command& t_keybind_map::operator[] (SDL_Scancode scan)
{
	auto i = m.find(scan);

	if (i == m.end())
		return empty_cmd;

	return i->second;
}

int keybinds_from_cfg (std::string path, t_keybind_map& m)
{
	std::ifstream f(path);

	if (!f.good())
		return -1;

	m.clear();

	std::string line;
	std::string key_name, bind;

	int line_nr = 0;

	while (true) {
		line_nr++;

		std::getline(f, line);

		if (!f.good())
			break;

		int comment_pos = line.find('#');
		if (comment_pos != std::string::npos)
			line.erase(comment_pos, std::string::npos);

		if (line.empty())
			continue;

		int colon_pos = line.find(':');
		if (colon_pos == std::string::npos)
			return line_nr;

		key_name = line.substr(0, colon_pos);
		bind = line.substr(colon_pos + 1, std::string::npos);

		SDL_Scancode scancode = SDL_GetScancodeFromName(key_name.c_str());
		if (scancode == SDL_SCANCODE_UNKNOWN)
			return line_nr;

		m[scancode] = parse_command(bind);
	}

	return 0;
}

}
