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
			run_command(key_binds[scan]);
			break;

		}
	}
}

t_command parse_command (std::string str)
{
	return { str };
}

void run_command (const t_command& cmd)
{
	if (cmd.cmd == "exit")
		core::due_to_quit = true;
}

}
