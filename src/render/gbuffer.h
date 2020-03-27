#ifndef GBUFFER_H
#define GBUFFER_H

#include "render/framebuffer.h"

extern t_fbo gbuf_fbo;

void init_gbuffers ();
void fill_gbuffers ();

void debug_show_gbuffers ();

constexpr int MRT_SLOT_WORLD_POS = 0;
constexpr int MRT_SLOT_WORLD_NORM = 1;

#endif // GBUFFER_H
