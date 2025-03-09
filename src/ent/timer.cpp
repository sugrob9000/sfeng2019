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
  ticks_left = kv.transform_with_default("time", stoi, 0);
  if (auto* val = kv.get("start-running")) {
    running = (*val == "1");
  } else {
    running = false;
  }
}