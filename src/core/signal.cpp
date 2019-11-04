#include "signal.h"

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
	DEBUG_EXPR(recipient_name);
	DEBUG_EXPR(signal_name);
	DEBUG_EXPR(argument);
}

}
