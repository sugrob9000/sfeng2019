#include "gl.h"
#include "inc_general.h"
#include <cassert>
#include <map>


void foo ()
{
	t_fbo fbo;
	fbo.make()
		.attach_color(make_tex2d_msaa(1024, 1024, GL_RGBA32F, 4), 0)
		.attach_color(make_rbo_msaa(1024, 1024, GL_RGB, 4), 1)
		.attach_depth(make_rbo(1024, 1024, GL_DEPTH_COMPONENT));
}



template <att_target_enum T>
void fbo_attach_lower (t_fbo&, const t_attachment<T>&, GLenum);
#define SPECIALIZE_ATTACH(type)                 \
	template <> void fbo_attach_lower<type> \
		(t_fbo& fbo, const t_attachment<type>& a, GLenum slot)

SPECIALIZE_ATTACH(tex2d)
{
	glFramebufferTexture2D(GL_FRAMEBUFFER, slot, GL_TEXTURE_2D, a.id, 0);
}

SPECIALIZE_ATTACH(tex2d_msaa)
{
	glFramebufferTexture2D(GL_FRAMEBUFFER, slot,
			GL_TEXTURE_2D_MULTISAMPLE, a.id, 0);
}

SPECIALIZE_ATTACH(rbo)
{
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, slot, GL_RENDERBUFFER, rbo);
}

SPECIALIZE_ATTACH(rbo_msaa)
{
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, slot, GL_RENDERBUFFER, rbo);
}


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
	fbo_assert_dimensions_equal(*this, att);
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	fbo_attach_lower<T>(*this, att, GL_COLOR_ATTACHMENT0 + idx);
	return *this;
}

template <att_target_enum T>
t_fbo& t_fbo::attach_depth (const t_attachment<T>& att)
{
	fbo_assert_dimensions_equal(*this, att);
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	fbo_attach_lower<T>(*this, att, GL_DEPTH_ATTACHMENT);
	return *this;
}

t_fbo& t_fbo::make ()
{
	glGenFramebuffers(1, &id);
	return *this;
}



std::pair<GLenum, GLenum> dissect_sized_type (GLenum);

void fbo_tex_params (GLenum target)
{
	// TODO: generalize this
	glTexParameteri(target, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

t_attachment<tex2d> make_tex2d (int w, int h, GLenum internal_type)
{
	t_attachment<tex2d> r;
	r.width = w;
	r.height = h;
	constexpr GLenum target = GL_TEXTURE_2D;
	auto comp_type = dissect_sized_type(internal_type);

	glGenTextures(1, &r.id);
	glBindTexture(target, r.id);
	fbo_tex_params(target);

	glTexImage2D(target, 0, internal_type,
		w, h, 0, comp_type.first, comp_type.second, nullptr);

	return r;
}

t_attachment<tex2d_msaa> make_tex2d_msaa (int w, int h,
		GLenum internal_type, int samples)
{
	t_attachment<tex2d_msaa> r;
	r.width = w;
	r.height = h;
	constexpr GLenum target = GL_TEXTURE_2D_MULTISAMPLE;

	glGenTextures(1, &r.id);
	glBindTexture(target, r.id);
	fbo_tex_params(target);
	glTexImage2DMultisample(target, samples, internal_type, w, h, GL_TRUE);

	return r;
}

t_attachment<rbo> make_rbo (int w, int h, GLenum internal_type)
{
	t_attachment<rbo> r;
	r.width = w;
	r.height = h;
	constexpr GLenum target = GL_RENDERBUFFER;
	glGenRenderbuffers(1, &r.id);
	glBindRenderbuffer(target, r.id);
	glRenderbufferStorage(target, internal_type, w, h);
	return r;
}

t_attachment<rbo_msaa> make_rbo_msaa (int w, int h,
		GLenum internal_type, int samples)
{
	t_attachment<rbo_msaa> r;
	r.width = w;
	r.height = h;
	constexpr GLenum target = GL_RENDERBUFFER;
	glGenRenderbuffers(1, &r.id);
	glBindRenderbuffer(target, r.id);
	glRenderbufferStorageMultisample(target, samples, internal_type, w, h);
	return r;
}


/*
 * Map an OpenGL sized type to a component enum and type enum pair,
 * e.g.
 * GL_RGBA32F => { GL_RGBA, GL_FLOAT }
 */
#define SIZED_TYPE(components, type, suffix) \
	{ GL_##components##suffix, { GL_##components, GL_##type }}
#define ALL_TYPES_FOR(c)                     \
	SIZED_TYPE(c, FLOAT, 32F),           \
	SIZED_TYPE(c, HALF_FLOAT, 16F),      \
	SIZED_TYPE(c, BYTE, 8I),             \
	SIZED_TYPE(c, UNSIGNED_BYTE, 8UI),   \
	SIZED_TYPE(c, SHORT, 16I),           \
	SIZED_TYPE(c, UNSIGNED_SHORT, 16UI), \
	SIZED_TYPE(c, INT, 32I),             \
	SIZED_TYPE(c, UNSIGNED_INT, 32UI),   \
	{ GL_##c, { GL_##c, GL_UNSIGNED_BYTE } }

static std::map<GLenum, std::pair<GLenum, GLenum>> sized_types =
	{ ALL_TYPES_FOR(R),
	  ALL_TYPES_FOR(RG),
	  ALL_TYPES_FOR(RGB),
	  ALL_TYPES_FOR(RGBA),
	  SIZED_TYPE(DEPTH_COMPONENT, FLOAT, 32F),
	  SIZED_TYPE(DEPTH_COMPONENT, HALF_FLOAT, 16),
	  { GL_DEPTH_COMPONENT, { GL_DEPTH_COMPONENT, GL_FLOAT } } };

std::pair<GLenum, GLenum> dissect_sized_type (GLenum sized)
{

	auto iter = sized_types.find(sized);
	if (iter == sized_types.end()) {
		fatal("Cannot convert sized type %x into components and type",
				sized);
	}
	return iter->second;
}


