#include "ent/trigger_sphere.h"
#include "core/signal.h"
#include "render/ctx.h"

template<>
void signal_handler<TriggerSphereEntity, SigTag("set_radius")>(TriggerSphereEntity& sphere, std::string arg) {
  float r = atof(arg.c_str());
  if (r > 0.0)
    sphere.radius = r;
}

void TriggerSphereEntity::think() {
  uint8_t touched_before = touching;

  vec3 delta = pos - camera.pos;
  float dist2 = delta.x * delta.x + delta.y * delta.y + delta.z * delta.z;
  touching = (dist2 <= radius * radius);

  switch ((uint8_t) touching | (touched_before << 1)) {
  case 0b10:
    on_event("exit");
    break;
  case 0b01:
    on_event("enter");
    break;
  default:
    // nothing changed
    break;
  }
}

void TriggerSphereEntity::apply_keyvals(const EntKeyvals& kv) {
  BaseEntity::apply_keyvals(kv);

  KV_TRY_GET(
    kv["radius"],
    {
      float r = atof(val.c_str());
      radius = (r > 0.0) ? r : DEFAULT_RADIUS;
    },
    radius = DEFAULT_RADIUS;
  );
}