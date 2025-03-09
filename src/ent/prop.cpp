#include "prop.h"
#include "core/signal.h"
#include "render/ctx.h"
#include "render/material.h"
#include "render/resource.h"

void PropEntity::apply_keyvals(const EntKeyvals& kv) {
  BaseEntity::apply_keyvals(kv);
  model = get_model(kv.get_with_default("model", "error"));
  material = kv.transform_with_default("mat", get_material, mat_none);
}

void PropEntity::do_render() const {
  mat4 restore = render_ctx.model;
  render_ctx.model = glm::translate(render_ctx.model, get_pos());
  render_ctx.model *= rotate_xyz_4x4(glm::radians(get_ang()));

  material->apply();
  model->render();

  render_ctx.model = restore;
}

Bbox PropEntity::get_bbox() const {
  // TODO: does not account for rotation
  Bbox r = model->bbox;
  r.start += get_pos();
  r.end += get_pos();
  return r;
}
