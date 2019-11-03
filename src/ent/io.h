#ifndef IO_H
#define IO_H

#include "inc_general.h"

namespace core
{

class e_base;
typedef void (*t_signal_handler) (e_base* _this, std::string arg);
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
};
/*
 * Keep a queue of signals, sorted by when they
 * are due to happen
 */
bool operator< (const t_signal& a, const t_signal& b);
extern std::priority_queue<t_signal> signals;

}

#endif // IO_H
