#ifndef ENT_PROP_H
#define ENT_PROP_H

#include "entity.h"
#include "render/render.h"

namespace core
{

ENT_DECL (prop)
{
	public:

	render::t_render_info* render_info;

	e_prop ();
};

}

#endif // ENT_PROP_H
