#include "entity.h"
#include "error.h"
#include "render/vis.h"
#include "ent/_headers.inc"

t_ent_registry ent_reg;

void e_base::set_name (const std::string& new_name)
{
	name = new_name;
	e_base* another = ents.find_by_name(new_name);
	if (another != nullptr) {
		warning("Entity at %p stole name %s from entity at %p",
				this, name.c_str(), another);
		another->set_name("");
	}
	ents.name_index[name] = this;
}

void e_base::on_event (const std::string& event) const
{
	auto i = events.find(event);
	if (i == events.end())
		return;
	for (const t_signal& s: i->second)
		add_signal(s);
}

void e_base::moved ()
{
	vis_requery_entity(this);
}

void e_base::apply_basic_keyvals (const t_ent_keyvals& kv)
{
	KV_TRY_GET(kv["pos"],
		atovec3(val, pos);,
		pos = vec3(); );
	KV_TRY_GET(kv["ang"],
		atovec3(val, ang);,
		ang = vec3(); );
	KV_TRY_GET(kv["name"],
		set_name(val),
		name = "");
}

void fill_ent_registry ()
{
	#define ENTITY(name)                             \
		ent_reg[#name] = &ent_factory<e_##name>; \
		fill_io_data<e_##name>();
	#include "ent/_list.inc"
	#undef ENTITY
}


t_entities ents;

e_base* t_entities::spawn (std::string type)
{
	f_ent_spawner spawner = ent_reg[type];
	if (spawner == nullptr)
		return nullptr;
	e_base* ent = spawner();
	vec.push_back(ent);
	return ent;
}

e_base* t_entities::find_by_name (std::string name)
{
	auto i = ents.name_index.find(name);
	if (i == ents.name_index.end())
		return nullptr;
	return i->second;
}
