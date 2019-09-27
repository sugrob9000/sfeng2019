#ifndef ENTITY_H
#define ENTITY_H

#include "inc_general.h"
#include <map>

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

typedef std::map<std::string, t_ent_new_routine> t_ent_registry;
extern t_ent_registry ent_registry;

void ent_fill_registry ();
e_base* ent_spawn (std::string type);

};

#include "ent_headers.inc"

#endif // ENTITY_H
