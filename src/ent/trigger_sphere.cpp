#include "ent/trigger_sphere.h"
#include "core/signal.h"
#include "render/ctx.h"

template<>
void signal_handler<e_trigger_sphere, SigTag("set_radius")>(e_trigger_sphere& sphere, std::string arg) {
  float r = atof(arg.c_str());
  if (r > 0.0)
    sphere.radius = r;
}

void e_trigger_sphere::think() {
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

void e_trigger_sphere::apply_keyvals(const t_ent_keyvals& kv) {
  e_base::apply_keyvals(kv);

  KV_TRY_GET(
    kv["radius"],
    {
      float r = atof(val.c_str());
      radius = (r > 0.0) ? r : DEFAULT_RADIUS;
    },
    radius = DEFAULT_RADIUS;
  );
}