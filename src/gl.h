#ifndef GL_H
#define GL_H

#include "inc_gl.h"

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

	constexpr static att_target_enum target = T;
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
};


#endif // GL_H
