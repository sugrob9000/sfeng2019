#ifndef ENTITY_H
#define ENTITY_H

#include "inc_general.h"
#include "keyval.h"
#include "signal.h"
#include "core.h"
#include "render/render.h"
#include <map>
#include <queue>

/*
 * The base entity class
 */
class e_base
{
	public:

	vec3 pos;
	vec3 ang;
	std::string name;

	e_base () { }
	virtual void think () = 0;

	virtual void apply_keyvals (const t_ent_keyvals& kv) = 0;
	/* Read pos, ang, and name. Your entity should probably do this. */
	void apply_basic_keyvals (const t_ent_keyvals& kv);

	t_eventmap events;
	/*
	 * We have to be able to get the sigmap knowing only the pointer
	 * to the entity, while in runtime - templates won't help with this
	 * The implementation is inserted by the preprocessor
	 */
	virtual const t_sigmap& get_sigmap () const = 0;
	void on_event (const std::string& event) const;
	void set_name (const std::string& name);

	virtual void render (t_render_stage s = SHADE_FINAL) const = 0;

	/*
	 * The entity promises that it is fully inside the box returned
	 * Entities that have no physical appearance (ie logical ones)
	 *   may express this by returning a box with volume 0
	 */
	virtual t_bound_box get_bbox () const = 0;

	/*
	 * Updates the engine's idea of where the entity is, for
	 * purposes such as vis. Call this on an entity whenever its
	 * output of get_bbox() changes
	 */
	void moved ();

	/* Used in vis to avoid redundant rendering */
	uint64_t render_last_guard_key;
};

/*
 * Goes inside the entity class declaration
 */
#define ENT_MEMBERS(name)                    \
	public:                              \
	void think ();                       \
	const t_sigmap& get_sigmap () const  \
		{ return sigmap<e_##name>; } \
	void render (t_render_stage) const;  \
	t_bound_box get_bbox () const;       \
	void apply_keyvals (const t_ent_keyvals& kv);


/*
 * Mapping entity class names (such as prop)
 * to C++ classes
 */
typedef e_base* (*f_ent_spawner) ();
typedef std::map<std::string, f_ent_spawner> t_ent_registry;
extern t_ent_registry ent_reg;
void fill_ent_registry ();

template <class e_derived>
e_base* ent_factory ()
{
	return new e_derived;
}


/*
 * A world's currently existing entities
 */
struct t_entities
{
	std::vector<e_base*> vec;
	std::map<std::string, e_base*> name_index;
	e_base* spawn (std::string type);
	e_base* find_by_name (std::string name);
};
extern t_entities ents;

#include "ent/ent_headers.inc"

#endif // ENTITY_H
