#ifndef LIGHT_SUN_H
#define LIGHT_SUN_H

#include "render/render.h"
#include "ent/lights.h"
#include <vector>

void init_lighting_sun ();
void compute_lighting_sun ();

extern std::vector<e_light_sun*> lights_sun;

namespace uniform_loc_light_sun
{
	constexpr int depth_map = 2;

	constexpr int light_rgb = 6;
	constexpr int light_view = 9;
	constexpr int light_dir = 25;
	constexpr int view_depths = 30;
}

#endif // LIGHT_SUN_H
