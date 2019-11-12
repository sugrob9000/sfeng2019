#ifndef ENT_TIMER_H
#define ENT_TIMER_H

#include "core/entity.h"
#include "core/core.h"

namespace core
{

class e_timer: public e_base
{
	public:

	bool running;
	long long ticks_left;

	ENT_MEMBERS (timer)
};

}

#endif // ENT_TIMER_H
