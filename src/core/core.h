#ifndef CORE_H
#define CORE_H

#include "inc_general.h"
#include "vec3.h"
#include "render.h"

namespace core
{

class t_entity
{
	public:

	vec3 pos;

	t_entity ();
};

extern bool due_to_quit;

void init ();

}

#endif // CORE_H

