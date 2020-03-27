#include "render/gbuffer.h"
#include "render/render.h"
#include "render/material.h"
#include "render/ctx.h"
#include "render/vis.h"
#include "input/cmds.h"

t_fbo gbuf_fbo;

void init_gbuffers ()
{
	int w = sdlctx.res_x;
	int h = sdlctx.res_y;

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

	glClear(GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	render_ctx.stage = G_BUFFERS;
	visible_set.render();
}

static bool show_gbuffers = false;
COMMAND_SET_BOOL (show_gbuf, show_gbuffers);

void debug_show_gbuffers ()
{
	if (show_gbuffers) {
		debug_render_texture(gbuf_fbo.color[MRT_SLOT_WORLD_POS]->id,
				0.5, 0.5, 0.5);
		debug_render_texture(gbuf_fbo.color[MRT_SLOT_WORLD_NORM]->id,
				0.5, 0.0, 0.5);
	}
}
