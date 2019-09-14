#include "input.h"
#include "keybind.h"

namespace input
{

void init (std::string input_conf_path)
{
	int input_status = keybinds_from_cfg(input_conf_path, key_binds);
	if (input_status != 0) {
		std::cerr << "Failed to initialze input: "
				<< input_status << '\n';
	}
}

void handle_input ()
{
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {

		case SDL_QUIT:
			core::due_to_quit = true;
			break;

		case SDL_KEYDOWN:
			SDL_Scancode scan = e.key.keysym.scancode;
			key_binds[scan].run();
			break;

		}
	}
}

t_command parse_command (std::string str)
{
	t_command ret;

	std::istringstream is(str);
	std::string arg;

	is >> ret.cmd;

	while (true) {
		is >> arg;
		if (!is.good())
			break;
		ret.args.push_back(arg);
	}

	return ret;
}

void t_command::run () const
{
	if (cmd == "exit")
		core::due_to_quit = true;
}

}
