#include "prop.h"
#include "render/resource.h"
#include "core/signal.h"

SIG_HANDLER (prop, setpos)
{
	atovec3(arg, ent->pos);
}

FILL_IO_DATA (prop)
{
	SET_SIG_HANDLER(prop, setpos);
}

void e_prop::think () { }

void e_prop::apply_keyvals (const t_ent_keyvals& kv)
{
	apply_basic_keyvals(kv);

	KV_TRY_GET(kv["model"],
		model = get_model(val);,
		model = get_model("error"); );
	KV_TRY_GET(kv["mat"],
		material = get_material(val);,
		material = get_material("error"); );
}

void e_prop::render () const
{
	glPushMatrix();

	material->apply();

	rotate_gl_matrix(ang);
	translate_gl_matrix(pos);

	model->render();

	glPopMatrix();
}

t_bound_box e_prop::get_bbox () const
{
	return model->bbox;
}
