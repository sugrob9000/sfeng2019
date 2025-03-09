#pragma once
#include "core/entity.h"

class e_timer: public e_base {
public:
  bool running;
  long long ticks_left;

  void think() override;
  void apply_keyvals(const t_ent_keyvals&) override;
  ENT_IMPLEMENT_GET_SIGMAP(e_timer);
};