#ifndef VIS_H
#define VIS_H

#include "inc_general.h"
#include <vector>

struct t_bound_box
{
	vec3 start;
	vec3 end;
	void render () const;
};

struct t_occlusion_plane
{
	std::vector<vec3> points;
};
extern std::vector<t_occlusion_plane> occ_planes;

class e_base;
extern std::vector<e_base*> visible_set;
void fill_visible_set ();

void init_vis ();

extern unsigned int occ_fbo_texture;

#endif // VIS_H
