#include "timer.h"

template<>
void signal_handler<TimerEntity, SigTag("start")>(TimerEntity& timer, std::string) {
  timer.running = true;
}

template<>
void signal_handler<TimerEntity, SigTag("stop")>(TimerEntity& timer, std::string) {
  timer.running = false;
}

template<>
void signal_handler<TimerEntity, SigTag("set")>(TimerEntity& timer, std::string arg) {
  timer.ticks_left = atoi(arg.c_str());
}

void TimerEntity::think() {
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

void TimerEntity::apply_keyvals(const EntKeyvals& kv) {
  BaseEntity::apply_keyvals(kv);

  KV_TRY_GET(kv["time"], ticks_left = atoi(val.c_str());, ticks_left = 0;);
  KV_TRY_GET(kv["start-running"], running = (val == "1");, running = false);
}