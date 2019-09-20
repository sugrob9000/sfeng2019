#ifndef CORE_H
#define CORE_H

#include "inc_general.h"
#include "vec3.h"
#include "render/render.h"

namespace core
{

class t_entity
{
	public:

	vec3 pos;
	render::t_render_info* render_info;

	void render ();
};

extern bool due_to_quit;

void init ();

void error (const std::string& msg, bool fatal);
void warn (const std::string& msg);

}

#endif // CORE_H

