#include "render/gbuffer.h"
#include "render/render.h"
#include "render/material.h"
#include "render/ctx.h"
#include "render/vis.h"

t_fbo gbuf_fbo;

void init_gbuffers ()
{
	int w = sdlcont.res_x;
	int h = sdlcont.res_y;

	gbuf_fbo.make()
		.attach_color(make_tex2d(w, h, GL_RGB32F), MRT_SLOT_WORLD_POS)
		.attach_color(make_tex2d(w, h, GL_RGB16F), MRT_SLOT_WORLD_NORM)
		.attach_depth(make_rbo(w, h, GL_DEPTH_COMPONENT))
		.assert_complete();

	sspace_add_buffer(gbuf_fbo);
}

void fill_gbuffers ()
{
	gbuf_fbo.apply();
	render_ctx.stage = G_BUFFERS;
	visible_set.render();
}
