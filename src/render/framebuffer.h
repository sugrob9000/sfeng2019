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
 *      .attach_color(make_tex2d(1024, 1024, GL_RGB), 1)
 *      .attach_color(make_tex2d(1024, 1024, GL_RGBA), 1)
 *      .attach_depth(make_rbo(1024, 1024, GL_DEPTH_COMPONENT));
 *
 *   ...
 *
 *   fbo.apply();
 *   // do the drawing, etc.
 */


/* The kinds of attachments possible */
enum att_target_enum {
	tex2d,
	tex2d_msaa,
	rbo,
	rbo_msaa
};

template <att_target_enum T>
struct t_attachment
{
	GLuint id;
	int width;
	int height;
};

t_attachment<tex2d> make_tex2d (int w, int h, GLenum internal_type);
t_attachment<tex2d_msaa> make_tex2d_msaa (int w, int h,
		GLenum internal_type, int samples);
t_attachment<rbo> make_rbo (int w, int h, GLenum internal_type);
t_attachment<rbo_msaa> make_rbo_msaa (int w, int h,
		GLenum internal_type, int samples);


struct t_fbo
{
	GLuint id;
	int width = 0;
	int height = 0;

	constexpr static int NUM_COLOR_ATTACHMENTS = 16;

	GLuint color[NUM_COLOR_ATTACHMENTS];
	GLuint depth;

	template <att_target_enum T>
	t_fbo& attach_color (const t_attachment<T>& att, int idx = 0);
	template <att_target_enum T>
	t_fbo& attach_depth (const t_attachment<T>& att);

	t_fbo& make ();
	void apply ();

	t_fbo& assert_complete ();
};


template <att_target_enum T>
void fbo_attach_lower (t_fbo&, const t_attachment<T>&, GLenum);
#define SPECIALIZE_ATTACH(type)                 \
	template <> void fbo_attach_lower<type> \
		(t_fbo& fbo, const t_attachment<type>& a, GLenum slot)

SPECIALIZE_ATTACH(tex2d);
SPECIALIZE_ATTACH(tex2d_msaa);
SPECIALIZE_ATTACH(rbo);
SPECIALIZE_ATTACH(rbo_msaa);

template <att_target_enum T>
inline void fbo_assert_dimensions_equal (t_fbo& fbo, const t_attachment<T>& a)
{
	if ((fbo.width != 0 && a.width != fbo.width)
	|| (fbo.height != 0 && a.height != fbo.height)) {
		fatal("Tried to attach object with dimensions %i, %i "
			"to FBO with dimensions %i, %i",
			a.width, a.height, fbo.width, fbo.height);
	}
	fbo.width = a.width;
	fbo.height = a.height;
}

template <att_target_enum T>
t_fbo& t_fbo::attach_color (const t_attachment<T>& att, int idx)
{
	assert(idx >= 0 && idx < NUM_COLOR_ATTACHMENTS);
	fbo_assert_dimensions_equal(*this, att);
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	fbo_attach_lower<T>(*this, att, GL_COLOR_ATTACHMENT0 + idx);
	color[idx] = att.id;
	return *this;
}

template <att_target_enum T>
t_fbo& t_fbo::attach_depth (const t_attachment<T>& att)
{
	fbo_assert_dimensions_equal(*this, att);
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	fbo_attach_lower<T>(*this, att, GL_DEPTH_ATTACHMENT);
	depth = att.id;
	return *this;
}
#endif // FRAMEBUFFER_H
