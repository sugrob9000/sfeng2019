#include "entity.h"

namespace core
{

void e_base::think () { }

template <class e_derived>
e_base* ent_new ()
{
	return new e_derived();
}

typedef e_base* (*t_ent_new_routine) ();

void init ()
{
}

};
