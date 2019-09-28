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
};
extern t_game game;

void init ();

}

#endif // CORE_H
