#pragma once
#include "base.h"
#include "core/signal.h"

class TriggerSphereEntity: public BaseEntity {
public:
  float radius;
  bool touching;

  ~TriggerSphereEntity() override = default;

  constexpr static float DEFAULT_RADIUS = 100.0;

  ENT_IMPLEMENT_GET_SIGMAP(TriggerSphereEntity);
  void think() override;
  void apply_keyvals(const EntKeyvals&) override;
};

template<>
inline void fill_io_data<TriggerSphereEntity>() {
  do_fill_io_data<TriggerSphereEntity, SigTag("set_radius")>();
}