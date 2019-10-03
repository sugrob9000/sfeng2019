#ifndef ENTITY_H
#define ENTITY_H

#include "inc_general.h"
#include <map>

namespace core
{

/*
 * Holds key/value pairs for entity initialization
 */
typedef std::map<std::string, std::string> t_ent_keyvals;

/*
 * The base entity class
 */
class e_base
{
	public:

	vec3 pos;
	vec3 ang;

	std::string name;

	virtual void think ();
	virtual void render () const;

	virtual void read (t_ent_keyvals& kv);
};

#define ENT_DECL(name) class e_##name: public e_base

template <class e_derived> e_base* ent_new ();
typedef e_base* (*t_ent_new_routine) ();


/*
 * The list of types of entities known to the engine
 */
class t_ent_registry
{
	public:

	std::map<std::string, t_ent_new_routine> m;

	t_ent_registry ();
	t_ent_new_routine& operator[] (std::string key);
};
extern t_ent_registry ent_registry;


/*
 * A world's currently existing entities
 */
class t_entities
{
	public:

	std::vector<e_base*> v;

	e_base* spawn (std::string type);
	e_base* find_by_name (std::string key);
};

};

#include "ent_headers.inc"

#endif // ENTITY_H
