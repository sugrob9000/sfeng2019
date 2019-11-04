#include "prop.h"
#include "render/resource.h"
#include "core/signal.h"

namespace core
{

ENT_CPP (prop)

SIG_HANDLER (prop, showcrd)
{
	DEBUG_EXPR(ent->pos);
}

FILL_IO_MAPS(prop)
{
	SET_SIG_HANDLER(prop, showcrd);
}


e_prop::e_prop () { }

void e_prop::think () { }

void e_prop::apply_keyvals (t_ent_keyvals& kv)
{
	KV_TRY_GET(kv["pos"],
		atovec3(val, pos);,
		pos = vec3(0.0, 0.0, 0.0); );
	KV_TRY_GET(kv["model"],
		model = render::get_model(val);,
		model = nullptr; );
	KV_TRY_GET(kv["mat"],
		material = render::get_material(val);,
		material = nullptr; );
}

void e_prop::render () const
{
	glPushMatrix();

	material->apply();
	glTranslatef(pos.x, pos.y, pos.z);
	model->render();

	glPopMatrix();
}

}
