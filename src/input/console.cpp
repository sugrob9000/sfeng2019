#include "console.h"
#include "cmds.h"
#include "render/render.h"
#include <algorithm>

bool console_active;

int cursor;
std::string cmd;
std::vector<const std::string*> matches;
std::string cmd_prefix;

void update_matches ();
void console_handle_input_ev (const SDL_Event& e)
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
			run_cmd_ext(cmd);
			cmd.clear();
			update_matches();
			break;

		case SDL_SCANCODE_TAB:
			if (!matches.empty()) {
				cmd = cmd_prefix + *matches[0] + ' ';
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
			console_close();
		break;
	case SDL_TEXTINPUT:
		cmd += e.text.text;
		update_matches();
		break;
	}
}

void console_open ()
{
	console_active = true;
	SDL_SetRelativeMouseMode(SDL_FALSE);
	SDL_StartTextInput();
	update_matches();
}

void console_close ()
{
	console_active = false;
	cmd.clear();
	SDL_SetRelativeMouseMode(SDL_TRUE);
	SDL_StopTextInput();
}


bool is_cmd_char (char c)
{
	return isalnum(c) || c == '_'
		|| c == '+' || c == '-';
}


void update_matches ()
{
	matches.clear();

	int n = cmd.length();
	const char* cmd_str = cmd.c_str();

	if (cmd_str[0] == '+' || cmd_str[0] == '-') {
		cmd_prefix = cmd_str[0];
		cmd_str++;
		n--;
	} else {
		cmd_prefix = "";
	}

	for (char c: cmd) {
		if (!is_cmd_char(c))
			return;
	}

	for (const auto& p: cmd_registry.m) {
		const std::string& s = p.first;
		if (n > s.length())
			continue;
		bool match = true;
		for (int i = 0; i < n; i++) {
			if (cmd_str[i] != s[i]) {
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

void console_render ()
{
	const SDL_Color bg_clr = { 20, 20, 20, 255 };
	const SDL_Color bg_match_clr = { 30, 30, 30, 240 };
	const SDL_Color cursor_clr = { 220, 220, 40, 255 };

	// try to match the actual font size in pixels,
	// and be 4 away from the top
	float text_height = sdlcont.font_h * 2.0 / sdlcont.res_y;
	float text_y = 4.0 * 2.0 / sdlcont.res_y;
	float height = text_height + 2.0 * text_y;

	float char_width = text_height
		* ((float) sdlcont.font_w / sdlcont.font_h)
		* ((float) sdlcont.res_y / sdlcont.res_x);
	float text_width = char_width * cmd.length();
	float text_x = char_width + 0.01;

	auto rect = [] (float x1, float y1, float x2, float y2)
	-> void {
		glBegin(GL_QUADS);
		glVertex2f(x1, y1);
		glVertex2f(x2, y1);
		glVertex2f(x2, y2);
		glVertex2f(x1, y2);
		glEnd();
	};

	glUseProgram(0);
	glColor4ubv((GLubyte*) &bg_clr);
	rect(-1.0, 1.0, 1.0, 1.0 - height);

	glColor4ubv((GLubyte*) &cursor_clr);
	glBegin(GL_LINES);
	glVertex2f(-1.0 + text_x + text_width, 1.0 - text_y);
	glVertex2f(-1.0 + text_x + text_width,
	            1.0 - text_y - text_height);
	glEnd();

	if (!matches.empty()) {
		float single_match_h = text_height + 0.015;
		float matches_h = single_match_h * matches.size();

		glColor4ubv((GLubyte*) &bg_match_clr);
		rect(-1.0, 1.0 - height, 1.0, 1.0 - height - matches_h);

		float matches_x = text_x + cmd_prefix.length() * char_width;

		for (int i = 0; i < matches.size(); i++) {
			draw_text(matches[i]->c_str(),
					-1.0 + matches_x,
					1.0 - height - i * single_match_h,
					char_width, text_height);
		}
	}

	if (!cmd.empty()) {
		draw_text(cmd.c_str(), -1.0 + text_x,
			1.0 - text_y, char_width, text_height);
	}
	draw_text(">", -1.0, 1.0 - text_y, char_width, text_height);
}

COMMAND_ROUTINE (console_open)
{
	if (ev == PRESS)
		console_open();
}

COMMAND_ROUTINE (console_close)
{
	if (ev == PRESS)
		console_close();
}
