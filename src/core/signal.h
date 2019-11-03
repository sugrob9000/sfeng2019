#ifndef SIGNAL_H
#define SIGNAL_H

#include "inc_general.h"
#include <queue>

namespace core
{

class e_base;
typedef void (*t_sig_handler) (e_base* ent, std::string arg);
typedef std::map<std::string, void*> t_iomap;

/*
 * Entities can be set to send signals
 * to one another
 */
struct t_signal
{
	std::string recipient_name;
	long long tick_due;
	std::string in_name;
	std::string argument;

	void execute () const;
};

/*
 * Keep a queue of signals, sorted by when they
 * are due to happen
 */
bool operator< (const t_signal& a, const t_signal& b);
extern std::priority_queue<t_signal> signals;

#define SIG_HANDLER(entclass, name) \
	void sig_##entclass##_##name (e_##entclass* ent, std::string arg)

}

#endif // SIGNAL_H
