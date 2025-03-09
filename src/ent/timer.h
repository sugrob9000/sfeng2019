#pragma once
#include "base.h"
#include "core/signal.h"

class e_timer: public e_base {
public:
  bool running;
  long long ticks_left;

  void think() override;
  void apply_keyvals(const t_ent_keyvals&) override;
  ENT_IMPLEMENT_GET_SIGMAP(e_timer);
};

template<>
inline void fill_io_data<e_timer>() {
  do_fill_io_data<e_timer,
    SigTag("start"),
    SigTag("stop"),
    SigTag("set")>();
}