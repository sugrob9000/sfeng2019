#include "timer.h"

SIG_HANDLER (timer, start)
{
	ent->running = true;
}

SIG_HANDLER (timer, stop)
{
	ent->running = false;
}

SIG_HANDLER (timer, set)
{
	ent->ticks_left = atoi(arg.c_str());
}

FILL_IO_DATA (timer)
{
	SET_SIG_HANDLER(timer, start);
	SET_SIG_HANDLER(timer, stop);
	SET_SIG_HANDLER(timer, set);
}

void e_timer::think ()
{
	if (running) {
		if (ticks_left <= 0) {
			running = false;
			on_event("time_up");
		} else {
			ticks_left--;
		}
	}
}

void e_timer::apply_keyvals (const t_ent_keyvals& kv)
{
	apply_basic_keyvals(kv);

	KV_TRY_GET(kv["time"],
		ticks_left = atoi(val.c_str());,
		ticks_left = 0; );
	KV_TRY_GET(kv["start-running"],
		running = (val == "1");,
		running = false);
}

void e_timer::render () const { }
