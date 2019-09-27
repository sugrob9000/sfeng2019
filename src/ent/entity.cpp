#include "entity.h"

namespace core
{

t_ent_registry ent_reg;

void e_base::think ()
{
	return;
}

template <class e_derived>
e_base* ent_new ()
{
	return new e_derived();
}

void ent_fill_registry ()
{
	#define ENT_REG(name) \
		ent_reg[#name] = &ent_new<e_##name>;
	#include "ent_list.inc"
	#undef ENT_REG
}

e_base* ent_spawn (std::string type)
{
	t_ent_new_routine spawner = ent_reg[type];
	if (spawner == nullptr)
		return nullptr;
	return spawner();
}

};
