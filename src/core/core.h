#ifndef CORE_H
#define CORE_H

#include "inc_general.h"
#include "vec3.h"
#include "render/render.h"
#include <queue>

namespace core
{

class t_entity
{
	public:

	vec3 pos;
	render::t_render_info* render_info;

	void render ();
};

struct t_signal
{
	long long tick_due;
	t_entity* source;
	t_entity* target;
};

/*
 * Compare signals by their due time
 */
bool operator< (const t_signal& a, const t_signal b);

class t_world
{
	public:

	std::vector<t_entity> entities;
	std::priority_queue<t_signal> signals;

	void render (render::t_camera& cam);
};


struct t_game_info
{
	bool must_quit;
	int exit_code;
};
extern t_game_info game;

void init ();

void error (const std::string& msg, bool fatal);
void warn (const std::string& msg);

}

#endif // CORE_H

