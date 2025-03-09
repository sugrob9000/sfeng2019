#pragma once
#include "core/core.h"
#include "core/entity.h"
#include "render/material.h"
#include "render/model.h"

class e_prop: public e_base {
public:
  t_model* model;
  t_material* material;

  void apply_keyvals(const t_ent_keyvals&) override;
  void render() const override;
  t_bound_box get_bbox() const override;
  ENT_IMPLEMENT_GET_SIGMAP(e_prop);
};