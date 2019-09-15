#include "input.h"
#include "keybind.h"
#include "cmds_basic.h"

namespace input
{

t_command_registry cmd_registry;

void init (std::string input_conf_path)
{
	int input_status = keybinds_from_cfg(input_conf_path, key_binds);
	if (input_status != 0) {
		std::cerr << "Failed to initialze input: "
				<< input_status << '\n';
	}

	cmd_registry.register_command("exit", &cmd::exit);
}

void handle_input ()
{
	SDL_Event e;
	while (SDL_PollEvent(&e)) {

		uint8_t kb_action;

		switch (e.type) {

		case SDL_QUIT: {
			core::due_to_quit = true;
			break;
		}

		case SDL_KEYDOWN: {
			kb_action = t_action::PRESS;
			// fall through
		}

		case SDL_KEYUP: {
			kb_action = t_action::RELEASE;

			SDL_Scancode scan = e.key.keysym.scancode;
			const t_command& cmd = key_binds[scan];
			cmd_registry.run(cmd, kb_action);
			break;
		}

		}
	}
}

t_command parse_command (std::string str)
{
	t_command ret;

	std::istringstream is(str);
	std::string arg;

	is >> ret.name;

	while (is >> arg)
		ret.args.push_back(arg);

	return ret;
}

void t_command_registry::register_command (
		std::string name,
		t_cmd_routine routine )
{
	m[name].routine = routine;
}

void t_command_registry::run (const t_command& cmd, uint8_t ev)
{
	t_action& action = m[cmd.name];
	t_cmd_routine& routine = action.routine;

	if (routine != nullptr)
		routine(cmd.args, ev);
}

}
