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

	/* Return this box, expanded to include other */
	t_bound_box updated (const t_bound_box& other) const;

	bool point_in (vec3 pt) const;
};

struct t_occlusion_plane
{
	std::vector<vec3> points;
};
extern std::vector<t_occlusion_plane> occ_planes;

void init_vis ();

void build_world_from_obj (std::string path);
void draw_octree ();

#endif // VIS_H
