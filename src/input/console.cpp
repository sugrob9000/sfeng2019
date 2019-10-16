#include "console.h"

namespace input
{

t_console_info console;

void t_console_info::handle_input ()
{
	SDL_Event e;
	while (SDL_PollEvent(&e)) {
		switch (e.type) {
		case SDL_QUIT:
			core::game.must_quit = true;
			break;
		case SDL_KEYDOWN:
			switch (e.key.keysym.scancode) {
			case SDL_SCANCODE_BACKSPACE:
				if (!cmd.empty())
					cmd.pop_back();
				break;
			case SDL_SCANCODE_RETURN:
				cmd_registry.run(parse_command(cmd), PRESS);
				cmd.clear();
				break;
			default:
				break;
			}
			break;
		// handle esc on keyup to avoid
		// sending the keyup event to the main game
		case SDL_KEYUP:
			if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE)
				close();
			break;
		case SDL_TEXTINPUT:
			cmd += e.text.text;
			break;
		}
	}
}

void t_console_info::open ()
{
	active = true;
	cmd.clear();
	SDL_SetRelativeMouseMode(SDL_FALSE);
	SDL_StartTextInput();
}

void t_console_info::close ()
{
	active = false;
	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_StopTextInput();
}

}
