#include "prop.h"
#include "render/resource.h"
#include "core/signal.h"

SIG_HANDLER (prop, setpos)
{
	atovec3(arg, ent->pos);
	ent->moved();
}

SIG_HANDLER (prop, setang)
{
	atovec3(arg, ent->ang);
}

FILL_IO_DATA (prop)
{
	SET_SIG_HANDLER(prop, setpos);
	SET_SIG_HANDLER(prop, setang);
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
		material = mat_none; );
}

void e_prop::render (t_render_stage s) const
{
	glPushMatrix();

	translate_gl_matrix(pos);
	rotate_gl_matrix(ang);

	material->apply(s);
	model->render();

	glPopMatrix();
}

t_bound_box e_prop::get_bbox () const
{
	// TODO: does not account for rotation
	t_bound_box r = model->bbox;
	r.start += pos;
	r.end += pos;
	return r;
}
