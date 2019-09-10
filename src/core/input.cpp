#include "input.h"
#include "core.h"

namespace input
{

keybind_map key_bindings;

void init (std::string input_conf_path)
{
	int input_status = keybinds_from_cfg(input_conf_path, key_bindings);
	if (input_status != 0) {
		std::cerr << "Failed to initialze input: "
				<< input_status << '\n';
	}
}

void handle_input ()
{

}

command parse_command (std::string str)
{
	return { str, {} };
}

void run_command (command& cmd)
{

}

int keybinds_from_cfg (std::string path, keybind_map& m)
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
