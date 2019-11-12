#include "console.h"
#include <algorithm>
#include "cmds.h"

t_console_info console;

void t_console_info::handle_input_ev (const SDL_Event& e)
{
	switch (e.type) {
	case SDL_KEYDOWN:
		switch (e.key.keysym.scancode) {
		case SDL_SCANCODE_BACKSPACE:

			if (cmd.empty())
				break;
			if (SDL_GetModState() & KMOD_SHIFT)
				cmd.clear();
			else
				cmd.pop_back();
			update_matches();
			break;

		case SDL_SCANCODE_RETURN:
			cmd_registry.run(parse_command(cmd), PRESS);
			cmd.clear();
			update_matches();
			break;

		case SDL_SCANCODE_TAB:
			if (!matches.empty()) {
				cmd = *matches[0] + ' ';
				update_matches();
			}
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
		update_matches();
		break;
	}
}

void t_console_info::open ()
{
	active = true;
	SDL_SetRelativeMouseMode(SDL_FALSE);
	SDL_StartTextInput();
	update_matches();
}

void t_console_info::close ()
{
	active = false;
	cmd.clear();
	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_StopTextInput();
}

void t_console_info::update_matches ()
{
	matches.clear();
	for (char c: cmd) {
		if (!isalnum(c) && c != '_')
			return;
	}
	for (const auto& p: cmd_registry.m) {
		const std::string& s = p.first;
		if (cmd.length() > s.length())
			continue;
		bool match = true;
		for (int i = 0; i < cmd.length(); i++) {
			if (cmd[i] != s[i]) {
				match = false;
				break;
			}
		}
		if (match)
			matches.push_back(&s);
	}
	std::sort(matches.begin(), matches.end(),
		[] (const std::string* a, const std::string* b) {
			return *a < *b;
		});
}

void t_console_info::render ()
{
	int resx = sdlcont.res_x;
	int resy = sdlcont.res_y;

	glPushMatrix();
	glScalef(2.0 / resx, -2.0 / resy, 1.0);
	glTranslatef(-0.5 * resx, -0.5 * resy, 0.0);

	constexpr int height = sdlcont.font_h + 10;
	int startx = sdlcont.font_w + 5;
	int starty = 4;

	const SDL_Color bg_clr = { 20, 20, 20, 255 };
	const SDL_Color bg_match_clr = { 30, 30, 30, 240 };
	const SDL_Color cursor_clr = { 220, 220, 40, 255 };

	glUseProgram(0);
	glBegin(GL_QUADS);
	glColor4ubv((GLubyte*) &bg_clr);
	glVertex2i(0, 0);
	glVertex2i(resx, 0);
	glVertex2i(resx, height);
	glVertex2i(0, height);
	glColor4ubv((GLubyte*) &cursor_clr);
	int x = cmd.size() * sdlcont.font_w + startx;
	glVertex2i(x, starty);
	glVertex2i(x + 2, starty);
	glVertex2i(x + 2, starty + sdlcont.font_h);
	glVertex2i(x, starty + sdlcont.font_h);
	glEnd();

	if (!cmd.empty())
		draw_text(cmd.c_str(), startx, starty);
	draw_text(">", 4, starty);

	if (!matches.empty()) {
		int single_match_h = sdlcont.font_h + 3;
		int matches_h = single_match_h * matches.size();

		glUseProgram(0);
		glBegin(GL_QUADS);
		glColor4ubv((GLubyte*) &bg_match_clr);
		glVertex2i(startx, height);
		glVertex2i(resx, height);
		glVertex2i(resx, height + matches_h);
		glVertex2i(startx, height + matches_h);
		glEnd();

		for (int i = 0; i < matches.size(); i++) {
			draw_text(matches[i]->c_str(), startx + 4,
					height + single_match_h * i);
		}
	}

	glPopMatrix();
}

COMMAND_ROUTINE (console_open)
{
	if (ev == PRESS)
		console.open();
}

COMMAND_ROUTINE (console_close)
{
	if (ev == PRESS)
		console.close();
}
