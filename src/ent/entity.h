#ifndef ENTITY_H
#define ENTITY_H

#include "inc_general.h"

namespace core
{

class e_base
{
	public:

	vec3 pos;
	vec3 ang;

	std::string name;

	virtual void think ();
};

#define ENT_DECL(name) \
	class e_##name: public e_base

#define ENT_REG(name) \
	class e_##name;

#include "entities.inc"
#undef ENT_REG

};

#endif // ENTITY_H
