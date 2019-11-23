#ifndef VIS_H
#define VIS_H

#include "inc_general.h"
#include <vector>

struct t_bound_box
{
	vec3 start;
	vec3 end;
	void render () const;

	float volume () const;

	/* Expand to include the argument */
	void update (vec3 pt);
	void update (const t_bound_box& other);

	bool point_in (vec3 pt) const;
};

void init_vis ();

void vis_initialize_world (std::string path);

extern int total_visible_nodes;
void draw_visible ();

#endif // VIS_H
