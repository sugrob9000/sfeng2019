#ifndef CORE_H
#define CORE_H

#include "inc_general.h"
#include "ent/entity.h"
#include <queue>

namespace core
{

/*
 * Pretty much a singleton to bundle the globals
 * related to the game logic together
 */
class t_game
{
	public:

	bool must_quit;
	int exit_code;
	long long tick;

	t_entities ents;

	t_game ();
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
};
extern t_game game;

void init ();

}

#endif // CORE_H
