#pragma once
#include "base.h"
#include "core/signal.h"

class TimerEntity: public BaseEntity {
public:
  bool running;
  long long ticks_left;

  ~TimerEntity() override = default;

  void think() override;
  void apply_keyvals(const EntKeyvals&) override;
  ENT_IMPLEMENT_GET_SIGMAP(TimerEntity);
};

template<>
inline void fill_io_data<TimerEntity>() {
  do_fill_io_data<TimerEntity,
    SigTag("start"),
    SigTag("stop"),
    SigTag("set")>();
}