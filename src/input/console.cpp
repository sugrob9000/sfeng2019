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

void t_console_info::render ()
{
	int resx = render::cont.res_x;
	int resy = render::cont.res_y;

	glPushMatrix();
	glScalef(2.0 / resx, -2.0 / resy, 1.0);
	glTranslatef(-0.5 * resx, -0.5 * resy, 0.0);

	constexpr int height = render::cont.font_h + 10;
	int startx = render::cont.font_w + 5;
	int starty = 4;

	const SDL_Color bg_clr = { 20, 20, 20, 255 };
	const SDL_Color cursor_clr = { 220, 220, 40, 255 };
	glBegin(GL_QUADS);
	glColor4ubv((GLubyte*) &bg_clr);
	glVertex2i(0, 0);
	glVertex2i(resx, 0);
	glVertex2i(resx, height);
	glVertex2i(0, height);

	glColor4ubv((GLubyte*) &cursor_clr);
	int x = cmd.size() * render::cont.font_w + startx;
	glVertex2i(x, starty);
	glVertex2i(x + 2, starty);
	glVertex2i(x + 2, starty + render::cont.font_h);
	glVertex2i(x, starty + render::cont.font_h);
	glEnd();

	render::draw_text(">", 4, starty);
	if (!cmd.empty())
		render::draw_text(cmd.c_str(), startx, starty);

	glPopMatrix();
}

}
