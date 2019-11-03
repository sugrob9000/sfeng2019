#include "entity.h"

namespace core
{

t_ent_registry ent_reg;

e_base::e_base () { }
void e_base::think () { }
void e_base::render () const { }

t_iomap& e_base::get_iomap () const
{
	core::fatal("Tried to get iomap of base entity class");
}

/*
 * Exemplar
 */
void e_base::apply_keyvals (t_ent_keyvals& kv)
{
	KV_TRY_GET(kv["pos"],
		atovec3(val, pos);,
		pos = vec3(); );
}


template <class e_derived>
e_base* ent_new ()
{
	return new e_derived();
}

void fill_ent_registry ()
{
	#define ENT_REG(name) \
		ent_reg[#name] = &ent_new<e_##name>;
	#include "ent/ent_list.inc"
	#undef ENT_REG
}

e_base* t_entities::spawn (std::string type)
{
	t_ent_spawner spawner = ent_reg[type];

	if (spawner == nullptr)
		return nullptr;

	e_base* ent = spawner();
	vec.push_back(ent);

	return ent;
}

};
