#ifndef SIGNAL_H
#define SIGNAL_H

#include "misc.h"
#include <queue>
#include <map>

/*
 * Entities can be sent signals to. Each entity class can implement
 *   handlers for its own signals, and set them up at engine startup.
 * For example, a door class may have a handler for
 *   "open" and "close" signals.
 *
 * A signal knows:
 *   - its target entity, by name
 *   - the tick on which it is due to happen
 *   - what signal for the target to execute and with which argument
 * Note that it does not know the class of which its target is,
 *   so invalid signals should be tolerated.
 *
 * Name resolution happens *when the delay is up*,
 *   not when the signal is sent.
 */

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
void add_signal (t_signal s);



/*
 * With these the entity can set up its own signal handlers.
 *
 * Each handler implementation should be declared with SIG_HANDLER.
 *   Note that these don't have to be present in any header.
 *
 * Then there must be a function declared with FILL_IO_DATA,
 *   which should call SET_SIG_HANDLER with entity name
 *   and the handler name. Again, FILL_IO_DATA doesn't have
 *   to be in a header, since it's prototyped as a template,
 *   and the entity implementation is only specializing it.
 */

/* Below are wrappers around this that should actually be used */
#define _SIGH_INTERNAL(entclass, name, proc) \
	sigmap<e_##entclass>[#name] = (f_sig_handler) proc;

/*
 * Declare or define a routine that is called once for your
 * entity class. It should register the hanglers, i.e. use SET_SIG_HANDLER
 */
#define FILL_IO_DATA(entclass) \
	template <> void fill_io_data<e_##entclass> ()

/* Declare or define a signal handler */
#define SIG_HANDLER(entclass, name) \
	void sig_##entclass##_##name (e_##entclass* ent, std::string arg)

/*
 * Register a signal handler, declared with SIG_HANDLER, for
 * an entity class - presumably used in a FILL_IO_DATA()
 */
#define SET_SIG_HANDLER(entclass, name) \
	_SIGH_INTERNAL(entclass, name, sig_##entclass##_##name)


/* Basic useful signal handlers. You entity should probably have them. */
#define BASIC_SIG_HANDLERS(entclass)                                 \
	do {                                                         \
		_SIGH_INTERNAL(entclass, setpos, sig_base_setpos);   \
		_SIGH_INTERNAL(entclass, setang, sig_base_setang);   \
		_SIGH_INTERNAL(entclass, setname, sig_base_setname); \
		_SIGH_INTERNAL(entclass, showpos, sig_base_showpos); \
	} while (0)

class e_base;
typedef void (*f_sig_handler) (e_base* ent, std::string arg);
typedef std::map<std::string, f_sig_handler> t_sigmap;

SIG_HANDLER (base, setpos);
SIG_HANDLER (base, setang);
SIG_HANDLER (base, setname);
SIG_HANDLER (base, showpos);

/*
 * Events: each entity object (as opposed to class) may specify
 *   on which events it wants to fire certain signals
 *   (maybe, several on one event).
 * For exmaple, a particular trigger volume may want to tell a
 *   particular door to open when someone steps in it.
 */

typedef std::map<std::string, std::vector<t_signal>> t_eventmap;

template <class entclass> void fill_io_data ();
template <class entclass> t_sigmap sigmap;

#endif // SIGNAL_H
