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


/*
 * With these the entity can set up its own signal handlers.
 *
 * Each handler implementation should be declared with
 *   SIG_HANDLER. Note that it doesn't have to be in the header
 *
 * Then there must be a function declared with FILL_IO_MAPS,
 *   which should call SET_SIG_HANDLER with entity name
 *   and the handler name. Again, FILL_IO_MAPS doesn't have
 *   to be in the header, since it's prototyped as a template.
 */

#define FILL_IO_MAPS(entclass) \
	template <> void fill_io_maps<e_##entclass> ()

#define SIG_HANDLER(entclass, name) \
	void sig_##entclass##_##name (e_##entclass* ent, std::string arg)

#define SET_SIG_HANDLER(entclass, name)                      \
	do {                                                 \
		iomap<e_##entclass>[#name] = (f_sig_handler) \
				&sig_##entclass##_##name;    \
	} while (false)

template <class entclass> void fill_io_maps ();
template <class entclass> t_iomap iomap;

}

#endif // SIGNAL_H
