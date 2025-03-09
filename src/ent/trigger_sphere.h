#pragma once
#include "base.h"
#include "core/signal.h"

class e_trigger_sphere: public e_base {
public:
  float radius;
  bool touching;

  constexpr static float DEFAULT_RADIUS = 100.0;

  ENT_IMPLEMENT_GET_SIGMAP(e_trigger_sphere);
  void think() override;
  void apply_keyvals(const t_ent_keyvals&) override;
};

template<>
inline void fill_io_data<e_trigger_sphere>() {
  do_fill_io_data<e_trigger_sphere, SigTag("set_radius")>();
}