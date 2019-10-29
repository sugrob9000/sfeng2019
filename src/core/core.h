#ifndef CORE_H
#define CORE_H

#include "inc_general.h"
#include "ent/entity.h"
#include <queue>

namespace core
{

extern bool must_quit;
extern int exit_code;
extern long long tick;

extern t_entities ents;

void update ();

/*
 * Map format:
 * a series of entities, each following the format:
 * entity-class
 * 	key1 value1
 * 	key2 value2
 *
 * etc.
 * The line with the entity class must not start with whitespace;
 * the lines with key-value pairs must start with whitespace.
 */
void load_map (std::string path);
void init (std::string conf);

}

#endif // CORE_H
