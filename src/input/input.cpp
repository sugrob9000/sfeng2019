#include "input.h"
#include "bind.h"
#include "cmds.h"
#include "console.h"

t_command_registry cmd_registry;
f_mousemove_routine mousemove_proc;

void init_input ()
{
	// register all commands
	#define COMMAND(name) \
		cmd_registry.register_command(#name, &cmd_##name);
	#include "cmds.inc"
	#undef COMMAND

	mousemove_proc = &mouse_mousemove_camera;
}

void run_argv_commands (int argc, const char* const* argv)
{
	while (--argc)
		cmd_registry.run(parse_command(*++argv), PRESS);
}

inline void handle_key (SDL_Scancode scan, uint8_t action)
{
	const t_command& cmd = key_binds[scan];
	cmd_registry.run(cmd, action);
}

void handle_input_ev (const SDL_Event& e)
{
	switch (e.type) {
	case SDL_KEYDOWN:
		if (!e.key.repeat)
			handle_key(e.key.keysym.scancode, PRESS);
		break;
	case SDL_KEYUP:
		handle_key(e.key.keysym.scancode, RELEASE);
		break;
	case SDL_MOUSEBUTTONDOWN:
		handle_key(scan_mouse[e.button.button], PRESS);
		break;
	case SDL_MOUSEBUTTONUP:
		handle_key(scan_mouse[e.button.button], RELEASE);
		break;
	case SDL_MOUSEMOTION:
		if (mousemove_proc != nullptr) {
			mousemove_proc(e.motion.xrel, e.motion.yrel,
					e.motion.x, e.motion.y);
		}
		break;
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
	}
}

void handle_input ()
{
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_QUIT:
			must_quit = true;
			break;
		case SDL_WINDOWEVENT:
			if (e.window.event == SDL_WINDOWEVENT_RESIZED) {
				int w = e.window.data1;
				int h = e.window.data2;
				resize_window(w, h);
			}
			break;
		default:
			if (console.active)
				console.handle_input_ev(e);
			else
				handle_input_ev(e);
			break;
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
		f_cmd_routine routine)
{
	m[name].routine = routine;
}

void t_command_registry::run (const t_command& cmd, uint8_t ev)
{
	auto i = m.find(cmd.name);
	if (i == m.end())
		return;

	t_action& action = i->second;
	f_cmd_routine& routine = action.routine;

	if (routine != nullptr)
		routine(cmd.args, ev);
}

void run_script (std::string path)
{
	std::ifstream f(path);
	if (!f) {
		warning("Script file %s could not be opened",
				path.c_str());
		return;
	}
	for (std::string line; std::getline(f, line); ) {
		if (!line.empty())
			cmd_registry.run(parse_command(line), PRESS);
	}
}

COMMAND_ROUTINE (exec)
{
	if (ev != PRESS)
		return;
	for (const std::string& s: args)
		run_script(s);
}
