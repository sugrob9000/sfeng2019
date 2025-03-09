#include "render/sky.h"
#include "render/ctx.h"
#include "render/material.h"
#include "render/render.h"
#include "render/resource.h"

static GLuint sky_program;

void init_sky() {
  sky_program = make_glsl_program({get_vert_shader("internal/sky"), get_frag_shader("internal/sky")});
}

void render_sky() {
  material_barrier();
  glUseProgram(sky_program);
  render_ctx.submit_viewproj();

  glDisable(GL_DEPTH_TEST);
  glCallList(cuboid_dlist_inwards);
  glEnable(GL_DEPTH_TEST);
}
