#include "inc_general.h"
#include "framebuffer.h"
#include <cassert>
#include <map>

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
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, slot, GL_RENDERBUFFER, a.id);
}

SPECIALIZE_ATTACH(rbo_msaa)
{
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, slot, GL_RENDERBUFFER, a.id);
}


t_fbo& t_fbo::make ()
{
	glGenFramebuffers(1, &id);
	return *this;
}

t_fbo& t_fbo::assert_complete ()
{
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (status != GL_FRAMEBUFFER_COMPLETE)
		fatal("Framebuffer %i incomplete: status %x", id, status);

	if (width <= 0 || height <= 0) {
		fatal("Framebuffer %i with invalid dimensions %i, %i",
				id, width, height);
	}

	return *this;
}

void t_fbo::apply ()
{
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	glViewport(0, 0, width, height);
}

/* ================= Making different attachments ================= */


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
