#include "signal.h"
#include "input/cmds.h"
#include "core.h"

namespace core
{

std::priority_queue<t_signal> signals;
bool operator< (const t_signal& a, const t_signal& b)
{
	return a.tick_due > b.tick_due;
}

void t_signal::execute () const
{
	e_base* e = ents.find_by_name(target);
	if (e == nullptr)
		return;

	const t_sigmap& sigmap = e->get_sigmap();
	auto i = sigmap.find(signal_name);

	if (i == sigmap.end())
		return;

	f_sig_handler routine = i->second;
	if (routine != nullptr)
		routine(e, argument);
}

void add_signal (t_signal s)
{
	s.tick_due += tick;
	signals.push(s);
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
	core::t_signal s = { recipient, delay, signame, sigarg };
	core::add_signal(s);
}
