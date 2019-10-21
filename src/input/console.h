#include "input.h"

namespace input
{

/*
 * A console window for entering commands
 */
struct t_console_info
{
	bool active;
	int cursor;
	std::string cmd;

	void handle_input ();
	void open ();
	void close ();
	void render ();
};
extern t_console_info console;

}
