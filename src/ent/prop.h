#ifndef ENT_PROP_H
#define ENT_PROP_H

#include "entity.h"
#include "render/render.h"
#include "render/model.h"
#include "render/material.h"

namespace core
{

ENT_DECL (prop)
{
	public:

	render::t_model* model;
	render::t_material* material;

	e_prop ();
	void think ();
	void render ();
};

}

#endif // ENT_PROP_H
