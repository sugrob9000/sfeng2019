#include "entity.h"

namespace core
{

void e_base::think () { }

template <class e_derived>
e_base* ent_new ()
{
	return new e_derived();
}

void init ()
{
}

};
