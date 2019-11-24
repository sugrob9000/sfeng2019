#include "console.h"
#include "cmds.h"
#include "render/render.h"
#include <algorithm>

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
	const SDL_Color bg_clr = { 20, 20, 20, 255 };
	const SDL_Color bg_match_clr = { 30, 30, 30, 240 };
	const SDL_Color cursor_clr = { 220, 220, 40, 255 };

	// try to match the actual font size in pixels,
	// and be 4 away from the top
	float text_height = sdlcont.font_h * 2.0 / sdlcont.res_y;
	float text_offs_y = 4.0 * 2.0 / sdlcont.res_y;
	float height = text_height + 2.0 * text_offs_y;

	float char_width = text_height
		* ((float) sdlcont.font_w / sdlcont.font_h)
		* ((float) sdlcont.res_y / sdlcont.res_x);
	float text_width = char_width * cmd.length();
	float text_offs_x = char_width + 0.01;

	glUseProgram(0);
	glBegin(GL_QUADS);
	glColor4ubv((GLubyte*) &bg_clr);
	glVertex2f(-1.0, -1.0);
	glVertex2f(1.0, -1.0);
	glVertex2f(1.0, -1.0 + height);
	glVertex2f(-1.0, -1.0 + height);
	glEnd();

	glBegin(GL_LINES);
	glColor4ubv((GLubyte*) &cursor_clr);
	glVertex2f(-1.0 + text_offs_x + text_width, -1.0 + text_offs_y);
	glVertex2f(-1.0 + text_offs_x + text_width,
	           -1.0 + text_offs_y + text_height);
	glEnd();

	if (!cmd.empty()) {
		draw_text(cmd.c_str(), -1.0 + text_offs_x,
			-1.0 + text_offs_y, char_width, text_height);
	}
	draw_text(">", -1.0, -1.0 + text_offs_y, char_width, text_height);

	if (!matches.empty()) {
		float single_match_h = text_height + 0.015;
		float matches_h = single_match_h * matches.size();

		glUseProgram(0);
		glBegin(GL_QUADS);
		glColor4ubv((GLubyte*) &bg_match_clr);
		glVertex2f(-1.0, -1.0 + height);
		glVertex2f(1.0, -1.0 + height);
		glVertex2f(1.0, -1.0 + height + matches_h);
		glVertex2f(-1.0, -1.0 + height + matches_h);
		glEnd();

		for (int i = 0; i < matches.size(); i++) {
			draw_text(matches[i]->c_str(), -1.0 + text_offs_x,
					-1.0 + height + single_match_h * i,
					char_width, text_height);
		}
	}
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
