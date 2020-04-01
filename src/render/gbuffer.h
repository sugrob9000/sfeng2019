#ifndef GBUFFER_H
#define GBUFFER_H

#include "render/framebuffer.h"

/*
 * G-buffer layout:
 *  depth             depth in screenspace
 *  color 0    RGB    position in worldspace
 *  color 1    RGB    normals in worldspace
 *  color 2    R      specular exponent
 */

extern t_fbo gbuf_fbo;

void init_gbuffers ();
void fill_gbuffers ();
void gbuffer_pass ();

void debug_show_gbuffers ();

constexpr int GBUF_SLOT_WORLD_POS = 0;
constexpr int GBUF_SLOT_WORLD_NORM = 1;
constexpr int GBUF_SLOT_SPECULAR = 2;

constexpr int UNIFORM_LOC_GBUFFER_WORLD_POS = 200;
constexpr int UNIFORM_LOC_GBUFFER_WORLD_NORM = 201;
constexpr int UNIFORM_LOC_GBUFFER_SCREEN_DEPTH = 202;
constexpr int UNIFORM_LOC_GBUFFER_SPECULAR = 203;

#endif // GBUFFER_H
