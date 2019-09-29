#include "inc_general.h"
#include "model.h"
#include "render.h"

namespace render
{

t_model::t_model (std::string objpath)
{
	load_obj(objpath);
}

void t_model::render () const
{
	glBegin(GL_TRIANGLES);
	for (const t_vertex v: verts) {
		glNormal3f(v.norm.x, v.norm.y, v.norm.z);
		glTexCoord2f(v.u, v.v);
		glVertex3f(v.pos.x, v.pos.y, v.pos.z);
	}
	glEnd();
}

bool t_model::load_obj (std::string path)
{
	std::ifstream f(path);
	if (!f)
		return false;

	int line_nr = 1;
	for (std::string line; std::getline(f, line); line_nr++) {
		// TODO
	}

	return true;
}

bool t_model::load_rvd (std::string path)
{
	return false;
}

}
