#include "prop.h"
#include "render/resource.h"

namespace core
{

e_prop::e_prop ()
{
}

void e_prop::think ()
{
	DEBUG_EXPR(pos);
	DEBUG_EXPR(model);
	DEBUG_EXPR(material);
}

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
		material = (render::t_material*) &material; );
}

void e_prop::render () const
{
	glPushMatrix();

	material->apply();
	glTranslatef(-pos.x, -pos.y, -pos.z);
	model->render();

	glPopMatrix();
}

}
