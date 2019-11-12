#ifndef ENT_PROP_H
#define ENT_PROP_H

#include "core/entity.h"
#include "render/render.h"
#include "render/model.h"
#include "render/material.h"

class e_prop: public e_base
{
	public:

	t_model* model;
	t_material* material;

	ENT_MEMBERS (prop)
};

#endif // ENT_PROP_H
