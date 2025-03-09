#pragma once
#include "base.h"
#include "core/core.h"
#include "render/material.h"
#include "render/model.h"

class PropEntity: public BaseEntity {
public:
  Model* model;
  Material* material;

  ~PropEntity() override = default;

  void apply_keyvals(const EntKeyvals&) override;
  void render() const override;
  Bbox get_bbox() const override;
  ENT_IMPLEMENT_GET_SIGMAP(PropEntity);
};