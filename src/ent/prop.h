#ifndef ENT_PROP_H
#define ENT_PROP_H

#include "core/entity.h"
#include "render/render.h"
#include "render/model.h"
#include "render/material.h"

namespace core
{

ENT_HEADER (prop)

class e_prop: public e_base
{
	public:

	render::t_model* model;
	render::t_material* material;

	ENT_MEMBERS (prop)
};

SIG_HANDLER (prop, showcrd);

}

#endif // ENT_PROP_H
