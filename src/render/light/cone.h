#pragma once
#include "ent/lights.h"
#include <vector>

namespace cone {
void init_lighting_cone();
void compute_lighting_cone();
extern std::vector<LightConeEntity*> global_cone_list;
}  // namespace cone

// GLSL uniform locations for calculating light
// when rendering actual geometry from a light's perspective
namespace uniform_loc_light_cone {
constexpr int depth_map = 2;

constexpr int light_pos = 6;
constexpr int light_rgb = 9;
constexpr int light_view = 12;
constexpr int light_bounds = 100;
}  // namespace uniform_loc_light_cone