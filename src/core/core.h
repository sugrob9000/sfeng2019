#ifndef CORE_H
#define CORE_H

#include "inc_general.h"
#include "core/entity.h"
#include <queue>

extern bool must_quit;
extern int exit_code;
extern long long tick;

void update ();

/*
 * Map format:
 * a series of entities, each following the format:
 * entity-class
 * 	key1 value1
 * 	key2 value2
 * 	! event-name delay target signal...
 * etc.
 * The line with the entity class must not start with whitespace;
 * the lines with key-value pairs must start with whitespace.
 * Those lines among key-value pairs which start with !
 *   are event handlers, which tell the entity to send a signal
 *   when something happens.
 */
void load_map (std::string path);
void init_core ();

#endif // CORE_H
