#ifndef SIGNAL_H
#define SIGNAL_H

#include "inc_general.h"
#include <queue>

namespace core
{

class e_base;
typedef void (*f_sig_handler) (e_base* ent, std::string arg);
typedef std::map<std::string, f_sig_handler> t_iomap;

/*
 * Entities can be set to send signals
 * to one another
 */
struct t_signal
{
	std::string recipient_name;
	long long tick_due;
	std::string signal_name;
	std::string argument;

	void execute () const;
};

/*
 * Keep a queue of signals, sorted by when they
 * are due to happen
 */
bool operator< (const t_signal& a, const t_signal& b);
extern std::priority_queue<t_signal> signals;

template <class e_derived>
void fill_io_maps ();

#define FILL_IO_MAPS(entclass) \
	template <> void fill_io_maps<e_##entclass> ()

#define SIG_HANDLER(entclass, name) \
	void sig_##entclass##_##name (e_##entclass* ent, std::string arg)

#define SET_SIG_HANDLER(entclass, name)                   \
	do {                                              \
		io_##entclass[#name] = (f_sig_handler)    \
				&sig_##entclass##_##name; \
	} while (false)

}

#endif // SIGNAL_H
