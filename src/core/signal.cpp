#include "signal.h"
#include "input/cmds.h"

namespace core
{

std::priority_queue<t_signal> signals;
bool operator< (const t_signal& a, const t_signal& b)
{
	return a.tick_due < b.tick_due;
}

void t_signal::execute () const
{
	DEBUG_EXPR(tick_due);
	DEBUG_EXPR(target);
	DEBUG_EXPR(signal_name);
	DEBUG_EXPR(argument);
}

void sig_add (std::string target, int delay,
		std::string name, std::string arg)
{
	signals.push({ target, delay, name, arg });
}

} // namespace core

COMMAND_ROUTINE (signal)
{
	if (ev != PRESS)
		return;

	if (args.size() < 3)
		return;

	const std::string& recipient = args[0];
	int delay = atoi(args[1].c_str());
	const std::string& signame = args[2];

	std::string sigarg;
	if (args.size() >= 4) {
		for (int i = 3; i < args.size(); i++) {
			sigarg += args[i];
			sigarg += " ";
		}
		sigarg.pop_back();
	}
	core::sig_add(recipient, delay, signame, sigarg);
}
