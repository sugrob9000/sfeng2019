#include "prop.h"
#include "render/resource.h"
#include "core/signal.h"

FILL_IO_DATA (prop)
{
	BASIC_SIG_HANDLERS(prop);
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

void e_prop::render () const
{
	mat4 restore = render_ctx.model;
	render_ctx.model = glm::translate(render_ctx.model, pos);
	rotate_xyz(render_ctx.model, glm::radians(ang));

	material->apply();
	model->render();

	render_ctx.model = restore;
}

t_bound_box e_prop::get_bbox () const
{
	// TODO: does not account for rotation
	t_bound_box r = model->bbox;
	r.start += pos;
	r.end += pos;
	return r;
}
