#include "timer.h"

template<>
void signal_handler<e_timer, SigTag("start")>(e_timer& timer, std::string) {
  timer.running = true;
}

template<>
void signal_handler<e_timer, SigTag("stop")>(e_timer& timer, std::string) {
  timer.running = false;
}

template<>
void signal_handler<e_timer, SigTag("set")>(e_timer& timer, std::string arg) {
  timer.ticks_left = atoi(arg.c_str());
}

void e_timer::think() {
  if (running) {
    if (ticks_left <= 0) {
      running = false;
      ticks_left = 0;
      on_event("time_up");
    } else {
      ticks_left--;
    }
  }
}

void e_timer::apply_keyvals(const t_ent_keyvals& kv) {
  e_base::apply_keyvals(kv);

  KV_TRY_GET(kv["time"], ticks_left = atoi(val.c_str());, ticks_left = 0;);
  KV_TRY_GET(kv["start-running"], running = (val == "1");, running = false);
}