#include "ent/lights.h"
#include "core/signal.h"
#include "misc.h"
#include "render/ctx.h"
#include "render/light/cone.h"
#include "render/light/sun.h"
#include "render/render.h"
#include <vector>

std::vector<LightConeEntity*> lights;

// ======================== e_light_cone code ========================

template<>
void signal_handler<LightConeEntity, SigTag("setcolor")>(LightConeEntity& light, std::string arg) {
  atovec3(arg, light.rgb);
}

template<>
void signal_handler<LightConeEntity, SigTag("setcone")>(LightConeEntity& light, std::string arg) {
  float cone = atof(arg.c_str());
  if (cone > 0.0 && cone < 180.0)
    light.cone_angle = cone;
}

LightConeEntity::LightConeEntity() {
  lights_cone.push_back(this);
}

LightConeEntity::~LightConeEntity() {
  std::erase(lights_cone, this);
}

void LightConeEntity::moved() {
  BaseEntity::moved();

  // update visible set
  view();
  vis.fill();
}

void LightConeEntity::apply_keyvals(const EntKeyvals& kv) {
  BaseEntity::apply_keyvals(kv);

  KV_TRY_GET(kv["cone"], cone_angle = atof(val.c_str());, cone_angle = 60.0;);
  KV_TRY_GET(kv["rgb"], atovec3(val, rgb);, rgb = vec3(0.5););

  KV_TRY_GET(kv["reach"], reach = atof(val.c_str());, reach = 2000.0;);
  KV_TRY_GET(kv["near"], near_plane = atof(val.c_str()), near_plane = 1.0;);
}

void LightConeEntity::view() const {
  using namespace glm;

  render_ctx.proj = perspective(radians(2.0f * cone_angle), 1.0f, near_plane, reach);
  render_ctx.view = rotate_xyz(radians(ang - vec3(90.0, 0.0, 0.0)));
  render_ctx.view = translate(render_ctx.view, -pos);
  render_ctx.model = mat4(1.0);

  render_ctx.eye_pos = pos;
}

// ======================== e_light_sun code ========================

template<>
void signal_handler<SunEntity, SigTag("setcolor")>(SunEntity& light, std::string arg) {
  atovec3(arg, light.rgb);
}

void SunEntity::apply_keyvals(const EntKeyvals& kv) {
  BaseEntity::apply_keyvals(kv);

  KV_TRY_GET(kv["distance"], distance = atof(val.c_str());, distance = 2000.0;);
  KV_TRY_GET(kv["rgb"], atovec3(val, rgb), rgb = vec3(0.5););
}

SunEntity::SunEntity() {
  lights_sun.push_back(this);
}

SunEntity::~SunEntity() {
  std::erase(lights_sun, this);
}
