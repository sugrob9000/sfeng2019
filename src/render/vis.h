#ifndef VIS_H
#define VIS_H

#include "inc_general.h"

/*
 * See vis.cpp for details on the implementation
 * of visibility determination
 */

void init_vis ();

void vis_initialize_world (std::string path);

extern int total_visible_nodes;

void draw_visible (const vec3& cam);

#endif // VIS_H
