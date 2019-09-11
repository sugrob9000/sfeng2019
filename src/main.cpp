#include "inc_general.h"
#include "render.h"
#include "core/core.h"
#include "core/input.h"

int main (int argc, char** argv)
{
	render::init(640, 480);
	input::init("res/cfg/input");

	SDL_Event ev;
	while (true) {
		while (SDL_PollEvent(&ev)) {
			switch (ev.type) {
			case SDL_KEYDOWN:
				using namespace input;
				run_command(
					key_binds[ev.key.keysym.scancode]);
				break;
			case SDL_QUIT:
				return 0;
			}
		}
	}

	return 0;
}

