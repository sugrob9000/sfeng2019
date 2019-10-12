#include "input.h"
#include "bind.h"
#include "cmds.h"

namespace input
{

t_command_registry cmd_registry;
t_mousemove_routine mousemove_proc;

void init (std::string input_conf_path)
{
	cmd_registry.register_command("nop", &cmd::nop);
	cmd_registry.register_command("exit", &cmd::exit);
	cmd_registry.register_command("echo", &cmd::echo);
	cmd_registry.register_command("bind", &cmd::bind);
	cmd_registry.register_command("stdincmd", &cmd::stdincmd);
	cmd_registry.register_command("move", &cmd::move);

	run_script(input_conf_path);

	mousemove_proc = &cmd::basic_mousemove;

	SDL_SetRelativeMouseMode(SDL_TRUE);
}

inline void handle_key (SDL_Scancode scan, uint8_t action)
{
	const t_command& cmd = key_binds[scan];
	cmd_registry.run(cmd, action);
}

void handle_input ()
{
	SDL_Event e;
	while (SDL_PollEvent(&e)) {

		switch (e.type) {

		case SDL_QUIT: {
			core::game.must_quit = true;
			break;
		}
		case SDL_KEYDOWN: {
			if (!e.key.repeat)
				handle_key(e.key.keysym.scancode, PRESS);
			break;
		}
		case SDL_KEYUP: {
			handle_key(e.key.keysym.scancode, RELEASE);
			break;
		}
		case SDL_MOUSEBUTTONDOWN: {
			handle_key(scan_mouse[e.button.button], PRESS);
			break;
		}
		case SDL_MOUSEBUTTONUP: {
			handle_key(scan_mouse[e.button.button], RELEASE);
			break;
		}
		case SDL_MOUSEWHEEL: {
			int y = e.wheel.y;
			SDL_Scancode scan = scan_mwheel_up;
			if (y < 0) {
				scan = scan_mwheel_down;
				y = -y;
			}
			for (int i = 0; i < y; i++)
				handle_key(scan, PRESS);
			break;
		}
		case SDL_MOUSEMOTION: {
			mousemove_proc(e.motion.xrel, e.motion.yrel,
			               e.motion.x, e.motion.y);
			break;
		}

		}
	}
}

t_command parse_command (std::string str)
{
	t_command ret;

	std::istringstream is(str);

	is >> ret.name;

	for (std::string arg; is >> arg; )
		ret.args.push_back(arg);

	return ret;
}

void t_command_registry::register_command (
		std::string name,
		t_cmd_routine routine)
{
	m[name].routine = routine;
}

void t_command_registry::run (const t_command& cmd, uint8_t ev)
{
	auto i = m.find(cmd.name);
	if (i == m.end())
		return;

	t_action& action = i->second;
	t_cmd_routine& routine = action.routine;

	if (routine != nullptr)
		routine(cmd.args, ev);
}

void run_script (std::string path)
{
	std::ifstream f(path);
	if (!f)
		return;
	for (std::string line; std::getline(f, line); ) {
		if (!line.empty())
			cmd_registry.run(parse_command(line), PRESS);
	}
}

}
