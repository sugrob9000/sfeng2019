#ifndef ENTITY_H
#define ENTITY_H

#include "inc_general.h"
#include "keyval.h"
#include <map>
#include <queue>
#include "io.h"

namespace core
{

/*
 * The base entity class
 */
class e_base
{
	public:

	vec3 pos;
	vec3 ang;

	std::string name;

	e_base ();
	virtual void think ();
	virtual void render () const;
	virtual void apply_keyvals (t_ent_keyvals& kv);

	virtual const t_iomap& get_iomap () const;
};

#define ENT_CLASS(name) class e_##name: public e_base

#define ENT_HEADER(name) \
	extern t_iomap io_##name;

#define ENT_CPP(name)                        \
	t_iomap io_##name;                   \
	const t_iomap& e_##name::get_iomap() const \
	{ return io_##name; }

#define ENT_MEMBERS(name)                       \
	public:                                 \
	e_##name ();                            \
	void think ();                          \
	void render () const;                   \
	void apply_keyvals (t_ent_keyvals& kv); \
	const t_iomap& get_iomap () const;      \
	static t_iomap io;


/*
 * Mapping entity class names (such as prop)
 * to C++ classes
 */
template <class e_derived> e_base* ent_new ();
typedef e_base* (*t_ent_spawner) ();
typedef std::map<std::string, t_ent_spawner> t_ent_registry;
extern t_ent_registry ent_reg;
void fill_ent_registry ();

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

}

#include "ent_headers.inc"

#endif // ENTITY_H
