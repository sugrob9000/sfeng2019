#pragma once
#include "core/entity.h"

class e_timer: public e_base
{
	public:

	bool running;
	long long ticks_left;

	ENT_MEMBERS (timer)
};