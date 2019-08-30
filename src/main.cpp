#include "inc.h"

int main (int argc, char** argv)
{
	SDL_Window *window;
	SDL_Renderer *renderer;

	SDL_Init(SDL_INIT_EVERYTHING);

	window = SDL_CreateWindow("engine",
			SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
			800, 600, SDL_WINDOW_ALLOW_HIGHDPI);

	renderer = SDL_CreateRenderer(window, -1, 0);

	SDL_SetRenderDrawColor(renderer, 120, 120, 200, 255);
	SDL_RenderClear(renderer);

	SDL_RenderPresent(renderer);

	SDL_Event event;

	bool quit = false;

	while (!quit) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT)
				quit = true;
		}
	}

	SDL_DestroyWindow(window);

	SDL_Quit();

	return 0;
}
