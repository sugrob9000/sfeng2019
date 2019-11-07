#include "prop.h"
#include "render/resource.h"
#include "core/signal.h"

namespace core
{

SIG_HANDLER (prop, setpos)
{
	atovec3(arg, ent->pos);
}

SIG_HANDLER (prop, showpos)
{
	DEBUG_EXPR(ent->pos);
}

FILL_IO_MAPS(prop)
{
	SET_SIG_HANDLER(prop, setpos);
	SET_SIG_HANDLER(prop, showpos);
}


e_prop::e_prop () { }

void e_prop::think () { }

void e_prop::apply_keyvals (const t_ent_keyvals& kv)
{
	apply_basic_keyvals(kv);

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

	render::rotate_gl_matrix(ang);
	glTranslatef(pos.x, pos.y, pos.z);
	model->render();

	glPopMatrix();
}

} // namespace core
