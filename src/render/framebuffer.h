#ifndef FRAMEBUFFER_H
#define FRAMEBUFFER_H

#include "inc_gl.h"
#include <cassert>

/*
 * Helper / abstraction for OpenGL framebuffers and their attachments.
 * Example of usage:
 *
 *   t_fbo fbo;
 *   fbo.make()
 *      .attach_color(make_tex2d(1024, 1024, GL_RGB), 0)
 *      .attach_color(make_tex2d(1024, 1024, GL_RGBA), 1)
 *      .attach_depth(make_rbo(1024, 1024, GL_DEPTH_COMPONENT));
 *   ...
 *   fbo.apply();
 *   // do the drawing, etc.
 *
 * Note that OpenGL requires all attachments in a FBO to be of
 * the same dimensions; this is only asserted at runtime
 */


/*
 * The kinds of attachments possible.
 * These do not biject into OpenGL target enums
 * (there isn't a GL_RENDERBUFFER_MULTISAMPLE, for instance)
 */
enum att_target_enum: uint8_t
{
	tex2d, tex2d_msaa,
	tex2d_array, tex2d_array_msaa,
	rbo, rbo_msaa,
};

/*
 * A particular attachment
 */
struct t_attachment
{
	t_attachment (int w, int h, att_target_enum tgt)
		: width(w), height(h), target(tgt) { }
	t_attachment (): id(-1) { }

	GLuint id;

	GLenum pixel_type_combined; /* GL_RGBA32F etc. */
	GLenum pixel_components; /* GL_RGBA etc. */
	GLenum pixel_type; /* GL_FLOAT etc. */

	int width = 0;
	int height = 0;

	int depth = 1; /* Only relevant in 3D targets */
	uint8_t samples = 1; /* Only relevant in MSAA targets */
	att_target_enum target;

	void update (int w, int h, int depth, int samples);
};

/*
 * Functions to make an attachment. Different attachments require different
 * parameters with which to be made - the number of samples,
 * 1/2/3 dimensions (TODO for 1D and 3D textures), etc.
 */
t_attachment make_tex2d (int w, int h, GLenum internal_type);
t_attachment make_tex2d_msaa (int w, int h, GLenum internal_type, int samples);
t_attachment make_tex2d_array (int w, int h, int d, GLenum internal_type);
t_attachment make_tex2d_array_msaa (int w, int h, int d,
		GLenum internal_type, int samples);
t_attachment make_rbo (int w, int h, GLenum internal_type);
t_attachment make_rbo_msaa (int w, int h, GLenum internal_type, int samples);


/*
 * In all functions, slice only matters when the target is 3D
 */

struct t_fbo
{
	GLuint id;
	int width = 0;
	int height = 0;

	constexpr static int NUM_COLOR_ATTACHMENTS = 16;

	t_attachment color[NUM_COLOR_ATTACHMENTS];
	t_attachment depth;

	t_fbo& make ();
	void bind () { glBindFramebuffer(GL_FRAMEBUFFER, id); }
	void apply () { bind(); glViewport(0, 0, width, height); }
	t_fbo& assert_complete ();

	t_fbo& attach_color (const t_attachment& att, int idx = 0,
	                     int slice = 0);
	t_fbo& attach_depth (const t_attachment& att, int slice = 0);
};


/*
 * Screenspace buffers: those which correspond to the screen, and must have
 * their resolution updated accordingly, somewhat automatically
 */

void add_sspace_buffer (t_fbo& fbo);
void resize_sspace_buffers (int w, int h);

#endif // FRAMEBUFFER_H
