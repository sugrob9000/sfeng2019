#ifndef ENT_PROP_H
#define ENT_PROP_H

#include "entity.h"
#include "render/render.h"
#include "render/model.h"
#include "render/material.h"

namespace core
{

ENT_HEADER (prop)
ENT_CLASS (prop)
{
	public:

	render::t_model* model;
	render::t_material* material;

	ENT_MEMBERS (prop)
};

}

#endif // ENT_PROP_H
