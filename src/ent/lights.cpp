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
  light.set_rgb(stovec3(arg));
}

template<>
void signal_handler<LightConeEntity, SigTag("setcone")>(LightConeEntity& light, std::string arg) {
  float cone = atof(arg.c_str());
  if (cone > 0.0 && cone < 180.0) {
    light.set_cone_angle(cone);
  }
}

LightConeEntity::LightConeEntity() {
  cone::global_cone_list.push_back(this);
}

LightConeEntity::~LightConeEntity() {
  std::erase(cone::global_cone_list, this);
}

void LightConeEntity::on_moved() {
  BaseEntity::on_moved();

  // update visible set
  view();
  vis.fill();
}

void LightConeEntity::apply_keyvals(const EntKeyvals& kv) {
  BaseEntity::apply_keyvals(kv);
  cone_angle = kv.transform_with_default("cone", stof, 60.0f);
  rgb = kv.transform_with_default("rgb", stovec3, vec3(0.5f));
  reach = kv.transform_with_default("reach", stof, 2000.0f);
  near_plane = kv.transform_with_default("near", stof, 1.0f);
}

void LightConeEntity::view() const {
  using namespace glm;

  render_ctx.proj = perspective(radians(2.0f * cone_angle), 1.0f, near_plane, reach);
  render_ctx.view = rotate_xyz(radians(get_ang() - vec3(90.0, 0.0, 0.0)));
  render_ctx.view = translate(render_ctx.view, -get_pos());
  render_ctx.model = mat4(1.0);

  render_ctx.eye_pos = get_pos();
}

// ======================== e_light_sun code ========================

template<>
void signal_handler<SunEntity, SigTag("setcolor")>(SunEntity& light, std::string arg) {
  atovec3(arg, light.rgb);
}

void SunEntity::apply_keyvals(const EntKeyvals& kv) {
  BaseEntity::apply_keyvals(kv);
  distance = kv.transform_with_default("distance", stof, 2000.0f);
  rgb = kv.transform_with_default("rgb", stovec3, vec3(0.5f));
}

SunEntity::SunEntity() {
  sun::global_sun_list.push_back(this);
}

SunEntity::~SunEntity() {
  std::erase(sun::global_sun_list, this);
}
