#ifndef LIGHT_SUN_H
#define LIGHT_SUN_H

#include "render/render.h"
#include "ent/lights.h"
#include <vector>

void init_lighting_sun ();
void compute_lighting_sun ();

extern std::vector<e_light_sun*> lights_sun;

#endif // LIGHT_SUN_H
