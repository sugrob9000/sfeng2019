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
 *
 * Note that OpenGL requires all attachments in a FBO to be of
 * the same dimensions; this requirement, while checked for,
 * is not properly reflected - TODO?
 */


/*
 * The kinds of attachments possible. These do not biject into
 * OpenGL target enums (there is not a GL_FRAMEBUFFER_MULTISAMPLE,
 * for instance). This list may be extended as more options are implemented
 */
enum att_target_enum
{
	tex2d, tex2d_msaa,
	tex2d_array, tex2d_array_msaa,
	rbo, rbo_msaa,
};

#include <type_traits>

/* A particular attachment */
template <att_target_enum T>
struct t_attachment
{
	GLuint id;
	int width;
	int height;
	/* Only relevant in 3D targets */
	int depth = 1;
};

/*
 * Functions to make an attachment. Different attachments require different
 * parameters with which to be made - the number of samples,
 * 1/2/3 dimensions (TODO for 1D and 3D textures), etc.
 */
t_attachment<tex2d> make_tex2d (int w, int h, GLenum internal_type);
t_attachment<tex2d_msaa> make_tex2d_msaa (int w, int h,
		GLenum internal_type, int samples);

t_attachment<tex2d_array> make_tex2d_array (int w, int h, int d,
		GLenum internal_type);
t_attachment<tex2d_array_msaa> make_tex2d_array_msaa (int w, int h, int d,
		GLenum internal_type, int samples);

t_attachment<rbo> make_rbo (int w, int h, GLenum internal_type);
t_attachment<rbo_msaa> make_rbo_msaa (int w, int h,
		GLenum internal_type, int samples);


/*
 * In all functions, slice only matters when the target is 3D
 */

struct t_fbo
{
	GLuint id;
	int width = 0;
	int height = 0;

	constexpr static int NUM_COLOR_ATTACHMENTS = 16;

	GLuint color[NUM_COLOR_ATTACHMENTS];
	GLuint depth;

	template <att_target_enum T>
	t_fbo& attach_color (const t_attachment<T>& att, int idx = 0,
	                     int slice = 0);

	template <att_target_enum T>
	t_fbo& attach_depth (const t_attachment<T>& att, int slice = 0);

	t_fbo& make ();
	void apply ();

	t_fbo& assert_complete ();
};


template <att_target_enum T>
void fbo_attach_lower (t_fbo& fbo, const t_attachment<T>& att,
		GLenum type, int slice);
#define _FBO_SPECIALIZE_ATTACH(type)                      \
	template <> void fbo_attach_lower<type>           \
		(t_fbo& fbo, const t_attachment<type>& a, \
		 GLenum slot, int slice)

_FBO_SPECIALIZE_ATTACH (tex2d);
_FBO_SPECIALIZE_ATTACH (tex2d_msaa);
_FBO_SPECIALIZE_ATTACH (tex2d_array);
_FBO_SPECIALIZE_ATTACH (tex2d_array_msaa);
_FBO_SPECIALIZE_ATTACH (rbo);
_FBO_SPECIALIZE_ATTACH (rbo_msaa);

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
t_fbo& t_fbo::attach_color (const t_attachment<T>& att, int idx, int slice)
{
	assert(idx >= 0 && idx < NUM_COLOR_ATTACHMENTS);
	fbo_assert_dimensions_equal(*this, att);
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	fbo_attach_lower<T>(*this, att, GL_COLOR_ATTACHMENT0 + idx, slice);
	color[idx] = att.id;
	return *this;
}

template <att_target_enum T>
t_fbo& t_fbo::attach_depth (const t_attachment<T>& att, int slice)
{
	fbo_assert_dimensions_equal(*this, att);
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	fbo_attach_lower<T>(*this, att, GL_DEPTH_ATTACHMENT, slice);
	depth = att.id;
	return *this;
}

#endif // FRAMEBUFFER_H
