#pragma once
#include "core/entity.h"
#include "render/material.h"
#include "render/model.h"

class e_prop: public e_base {
public:
  t_model* model;
  t_material* material;

  ENT_MEMBERS(prop)
};