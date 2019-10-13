#include "entity.h"

namespace core
{

t_ent_registry ent_reg;

e_base::e_base () { }
void e_base::think () { }
void e_base::render () const { }

/*
 * Mostly an exemplar, since any entity
 * should really implement the reading of itself
 */
void e_base::apply_keyvals (t_ent_keyvals& kv)
{
	KV_TRY_GET(kv["pos"],
		atovec3(val, pos);,
		pos = vec3(0.0, 0.0, 0.0); );
}


template <class e_derived>
e_base* ent_new ()
{
	return new e_derived();
}

t_ent_registry::t_ent_registry ()
{
	#define ENT_REG(name) \
		m[#name] = &ent_new<e_##name>;
	#include "ent_list.inc"
	#undef ENT_REG
}

t_ent_new_routine& t_ent_registry::operator[] (std::string key)
{
	return m[key];
}

e_base* t_entities::spawn (std::string type)
{
	t_ent_new_routine spawner = ent_reg[type];

	if (spawner == nullptr)
		return nullptr;

	e_base* ent = spawner();
	v.push_back(ent);

	return ent;
}

};
