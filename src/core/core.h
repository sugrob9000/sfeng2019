#ifndef CORE_H
#define CORE_H

#include "inc_general.h"
#include "vec3.h"
#include "render/render.h"
#include <queue>

namespace core
{

class t_game_info
{
	public:

	bool must_quit;
	int exit_code;
	long long tick;

	t_game_info ();
	void update ();
};
extern t_game_info game;

void init ();

}

#endif // CORE_H

