#pragma once
#include "ent/lights.h"
#include <vector>

namespace sun {
void init_lighting_sun();
void compute_lighting_sun();
extern std::vector<SunEntity*> global_sun_list;
} //  namespace sun

namespace uniform_loc_light_sun {
constexpr int depth_map = 2;

constexpr int light_rgb = 6;
constexpr int light_view = 9;
constexpr int light_dir = 25;
constexpr int view_depths = 30;
}  // namespace uniform_loc_light_sun