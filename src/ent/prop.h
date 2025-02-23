#pragma once
#include "core/entity.h"
#include "render/model.h"
#include "render/material.h"

class e_prop: public e_base
{
	public:

	t_model* model;
	t_material* material;

	ENT_MEMBERS (prop)
};