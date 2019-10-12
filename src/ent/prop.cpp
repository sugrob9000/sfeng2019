#include "prop.h"
#include "render/resource.h"

namespace core
{

e_prop::e_prop ()
{
}

void e_prop::think ()
{
}

void e_prop::apply_keyvals (t_ent_keyvals& kv)
{
	atovec3(kv["pos"], pos);
	model = render::get_model(kv["model"]);
	material = render::get_material(kv["mat"]);
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
