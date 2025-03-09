#pragma once
#include "base.h"
#include "core/signal.h"
#include "render/render.h"
#include "render/vis.h"

// Directional cone light
class LightConeEntity: public BaseEntity {
public:
  float cone_angle;
  float near_plane;
  float reach;
  vec3 rgb;

  VisibleSet vis;

  LightConeEntity();
  ~LightConeEntity() override;

  void apply_keyvals(const EntKeyvals&) override;
  void moved() override;
  ENT_IMPLEMENT_GET_SIGMAP(LightConeEntity);

  void view() const;
};

template<>
inline void fill_io_data<LightConeEntity>() {
  do_fill_io_data<LightConeEntity, SigTag("setcone"), SigTag("setcolor")>();
}

// Parallel omnipresent light i.e. a sun
// World position on this doesn't change anything about its lighting
class SunEntity: public BaseEntity {
public:
  vec3 rgb;
  // Consider the sun to be this far from the eye, in the (opposite)
  //   direction of the rays. If there's something in the map farther
  //   than this, it will not cast shadows
  // Smaller values mean better depth buffer precision, though
  float distance;

  SunEntity();
  ~SunEntity() override;

  ENT_IMPLEMENT_GET_SIGMAP(SunEntity);
  void apply_keyvals(const EntKeyvals&) override;
};

template<>
inline void fill_io_data<SunEntity>() {
  do_fill_io_data<SunEntity, SigTag("setcolor")>();
}