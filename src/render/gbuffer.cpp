#include "render/gbuffer.h"
#include "render/render.h"
#include "render/material.h"
#include "render/ctx.h"
#include "render/vis.h"
#include "render/debug.h"
#include "input/cmds.h"

t_fbo gbuf_fbo;

void init_gbuffers ()
{
	int w = sdlctx.res_x;
	int h = sdlctx.res_y;

	gbuf_fbo.make()
		.attach_color(make_tex2d(w, h, GL_RGB32F), GBUF_SLOT_WORLD_POS)
		.attach_color(make_tex2d(w, h, GL_RGB16F),
				GBUF_SLOT_WORLD_NORM)
		.attach_color(make_tex2d(w, h, GL_R16F), GBUF_SLOT_SPECULAR)
		.attach_depth(make_tex2d(w, h, GL_DEPTH_COMPONENT24))
		.assert_complete();

	sspace_add_buffer(gbuf_fbo);
}

void fill_gbuffers ()
{
	gbuf_fbo.apply();

	glClearColor(0.0, 0.0, 0.0, 1.0);
	glClearDepth(1.0);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_BLEND);

	render_ctx.stage = RENDER_STAGE_G_BUFFERS;
	visible_set.render();
}

void gbuffer_pass ()
{
	glBegin(GL_QUADS);
	glVertex2i(-1, -1);
	glVertex2i(1, -1);
	glVertex2i(1, 1);
	glVertex2i(-1, 1);
	glEnd();
}

static int show_gbuffer = -1;
COMMAND_ROUTINE (show_gbuf)
{
	if (ev == RELEASE) {
		show_gbuffer = -1;
		return;
	}
	if (args.empty())
		return;
	show_gbuffer = atoi(args[0].c_str());
}

void debug_show_gbuffers ()
{
	if (show_gbuffer >= 0) {
		debug_render_tex2d(gbuf_fbo.color[show_gbuffer]->id,
				-1.0, -1.0, 2.0);
	}
}
