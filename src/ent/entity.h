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

#define ENT_DECL(name) class e_##name: public e_base

template <class e_derived> e_base* ent_new ();
typedef e_base* (*t_ent_new_routine) ();

};

#include "ent_headers.inc"

#endif // ENTITY_H
