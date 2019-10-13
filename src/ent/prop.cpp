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

	// output axes at origin
	glDisable(GL_DEPTH_TEST);
	glUseProgram(0);
	glBegin(GL_TRIANGLES);
	const float length = 25.0;
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(1.0, 1.0, 1.0);
	glVertex3f(length, 0.0, 0.0);
	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(1.0, 1.0, 1.0);
	glVertex3f(0.0, length, 0.0);
	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(0.0, 0.0, 0.0);
	glVertex3f(1.0, 1.0, 1.0);
	glVertex3f(0.0, 0.0, length);
	glEnd();
	glEnable(GL_DEPTH_TEST);

	glPopMatrix();
}

}
