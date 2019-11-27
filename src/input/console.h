#include "input.h"

/*
 * A console window for entering commands
 * Enter - execute
 * Esc - get out
 * Shift-backspace - erase current command
 */

void console_handle_input_ev (const SDL_Event& e);
void console_open ();
void console_close ();
void console_render ();

extern bool console_active;
