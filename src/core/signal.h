#ifndef SIGNAL_H
#define SIGNAL_H

#include "inc_general.h"
#include <queue>
#include <map>

/*
 * Entities can be sent signals to. Each entity class can implement
 *   handlers for its own signals, and set them up at engine startup.
 *
 * A signal can specify:
 *   - its target entity, by name
 *   - the tick on which it is due to happen
 *   - what signal for the target to execute and with which argument
 *
 * Name resolution happens *after* the delay is up.
 */

namespace core
{

class e_base;

/*
 * Let each class map signal names to handlers
 */
typedef void (*f_sig_handler) (e_base* ent, std::string arg);
typedef std::map<std::string, f_sig_handler> t_iomap;

struct t_signal
{
	std::string target;
	long long tick_due;
	std::string signal_name;
	std::string argument;

	void execute () const;
};

/*
 * Keep a queue of signals, sorted by when they
 * are due to happen, ascending
 */
bool operator< (const t_signal& a, const t_signal& b);
extern std::priority_queue<t_signal> signals;

/*
 * The basic routine used to fire a signal
 */
void sig_add (std::string recipient, int delay,
		std::string name, std::string arg);

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
