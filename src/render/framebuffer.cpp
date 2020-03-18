#include "misc.h"
#include "framebuffer.h"
#include <cassert>
#include <map>

static void assert_dimensions_equal (t_fbo& fbo, const t_attachment& att)
{
	if ((fbo.width != 0 && att.width != fbo.width)
	|| (fbo.height != 0 && att.height != fbo.height)) {
		fatal("Tried to attach object with dimensions %i, %i "
			"to FBO with dimensions %i, %i",
			att.width, att.height, fbo.width, fbo.height);
	}
	fbo.width = att.width;
	fbo.height = att.height;
}

static void attach_low (t_fbo& fbo, const t_attachment& att,
			GLenum slot, int slice)
{
	switch (att.target) {
	case tex2d:
		glFramebufferTexture2D(GL_FRAMEBUFFER, slot,
			GL_TEXTURE_2D, att.id, 0);
		break;
	case tex2d_msaa:
		glFramebufferTexture2D(GL_FRAMEBUFFER, slot,
			GL_TEXTURE_2D_MULTISAMPLE, att.id, 0);
		break;
	case tex2d_array:
		glFramebufferTextureLayer(GL_FRAMEBUFFER, slot,
			att.id, 0, slice);
		break;
	case tex2d_array_msaa:
		glFramebufferTexture3D(GL_FRAMEBUFFER, slot,
			GL_TEXTURE_2D_MULTISAMPLE_ARRAY, att.id, 0, slice);
		break;
	case rbo:
	case rbo_msaa:
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, slot,
			GL_RENDERBUFFER, att.id);
		break;
	default:
		fatal("FBO %i: Invalid target enum %i", att.id, att.target);
	}
}

t_fbo& t_fbo::attach_color (const t_attachment& att, int idx, int slice)
{
	assert(idx >= 0 && idx < NUM_COLOR_ATTACHMENTS);
	assert_dimensions_equal(*this, att);
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	color[idx] = att;
	attach_low(*this, att, GL_COLOR_ATTACHMENT0 + idx, slice);
	return *this;
}

t_fbo& t_fbo::attach_depth (const t_attachment& att, int slice)
{
	assert_dimensions_equal(*this, att);
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	attach_low(*this, att, GL_DEPTH_ATTACHMENT, slice);
	depth = att;
	return *this;
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

void t_attachment::update (int w, int h, int new_depth, int new_samples)
{
	width = w;
	height = h;
	depth = new_depth;
	samples = new_samples;

	switch (target) {
	case tex2d:
		glTexImage2D(GL_TEXTURE_2D, 0, pixel_type_combined, w, h, 0,
				pixel_type, pixel_components, nullptr);
		break;
	case tex2d_msaa:
		glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE,
				samples, pixel_type_combined, w, h, GL_TRUE);
		break;
	case tex2d_array:
		glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, pixel_type_combined,
				w, h, depth, 0, pixel_components,
				pixel_type, nullptr);
		break;
	case tex2d_array_msaa:
		glTexImage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY,
				samples, pixel_type_combined, w, h, depth, GL_TRUE);
		break;
	case rbo:
		glRenderbufferStorage(GL_RENDERBUFFER,
				pixel_type_combined, w, h);
		break;
	case rbo_msaa:
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples,
				pixel_type_combined, w, h);
		break;
	default:
		fatal("Attachment %i: Invalid target enum %i", id, target);
	}
}

/*
 * ================= Making different attachments =================
 */

std::pair<GLenum, GLenum> dissect_sized_type (GLenum);
static inline void store_pixel_type (t_attachment& att, GLenum combined)
{
	auto p = dissect_sized_type(combined);
	att.pixel_type = p.second;
	att.pixel_components = p.first;
	att.pixel_type_combined = combined;
}

t_attachment make_tex2d (int w, int h, GLenum internal_type)
{
	t_attachment r(w, h, tex2d);
	store_pixel_type(r, internal_type);

	constexpr GLenum target = GL_TEXTURE_2D;
	glGenTextures(1, &r.id);
	glBindTexture(target, r.id);

	glTexParameteri(target, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage2D(target, 0, internal_type, w, h, 0,
			r.pixel_components, r.pixel_type, nullptr);

	return r;
}

t_attachment make_tex2d_msaa (int w, int h, GLenum internal_type, int samples)
{
	t_attachment r(w, h, tex2d_msaa);
	store_pixel_type(r, internal_type);
	r.samples = samples;

	constexpr GLenum target = GL_TEXTURE_2D_MULTISAMPLE;

	glGenTextures(1, &r.id);
	glBindTexture(target, r.id);

	glTexImage2DMultisample(target, samples,
			r.pixel_type_combined, w, h, GL_TRUE);

	return r;
}

t_attachment make_tex2d_array (int w, int h, int d, GLenum internal_type)
{
	t_attachment r(w, h, tex2d_array);
	r.depth = d;
	store_pixel_type(r, internal_type);

	constexpr GLenum target = GL_TEXTURE_2D_ARRAY;

	glGenTextures(1, &r.id);
	glBindTexture(target, r.id);

	glTexParameteri(target, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTexImage3D(target, 0, internal_type, w, h, d, 0,
			r.pixel_components, r.pixel_type, nullptr);

	return r;
}

t_attachment make_tex2d_array_msaa (int w, int h, int d,
		GLenum internal_type, int samples)
{
	t_attachment r(w, h, tex2d_array_msaa);
	store_pixel_type(r, internal_type);
	r.depth = d;
	r.samples = samples;

	constexpr GLenum target = GL_TEXTURE_2D_MULTISAMPLE_ARRAY;

	glGenTextures(1, &r.id);
	glBindTexture(target, r.id);

	glTexImage3DMultisample(target, samples,
			r.pixel_type_combined, w, h, d, GL_TRUE);

	return r;
}

t_attachment make_rbo (int w, int h, GLenum internal_type)
{
	t_attachment r(w, h, rbo);
	store_pixel_type(r, internal_type);

	constexpr GLenum target = GL_RENDERBUFFER;
	glGenRenderbuffers(1, &r.id);
	glBindRenderbuffer(target, r.id);
	glRenderbufferStorage(target, r.pixel_type_combined, w, h);
	return r;
}

t_attachment make_rbo_msaa (int w, int h,
		GLenum internal_type, int samples)
{
	t_attachment r(w, h, rbo_msaa);
	store_pixel_type(r, internal_type);
	r.samples = samples;

	constexpr GLenum target = GL_RENDERBUFFER;
	glGenRenderbuffers(1, &r.id);
	glBindRenderbuffer(target, r.id);
	glRenderbufferStorageMultisample(target, samples,
			r.pixel_type_combined, w, h);
	return r;
}

/* ========================================================= */

/*
 * Map an OpenGL sized type to a component enum and type enum pair,
 * e.g.
 * GL_RGBA32F => { GL_RGBA, GL_FLOAT }
 */
#define SIZED_TYPE(components, type, suffix) \
	{ GL_##components##suffix, { GL_##components, GL_##type } }
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
