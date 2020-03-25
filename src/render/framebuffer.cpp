#include "misc.h"
#include "render/framebuffer.h"
#include <cassert>
#include <map>
#include <set>
#include <vector>


t_fbo& t_fbo::make ()
{
	glGenFramebuffers(1, &id);
	glBindFramebuffer(GL_FRAMEBUFFER, id);

	GLenum buffers[num_clr_attachments];
	for (int i = 0; i < num_clr_attachments; i++)
		buffers[i] = GL_COLOR_ATTACHMENT0 + i;
	glDrawBuffers(num_clr_attachments, buffers);

	return *this;
}

t_fbo& t_fbo::assert_complete ()
{
	glBindFramebuffer(GL_FRAMEBUFFER, id);
	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

	if (width <= 0 || height <= 0) {
		fatal("Framebuffer %i with invalid dimensions %i, %i",
				id, width, height);
	}

	if (status != GL_FRAMEBUFFER_COMPLETE)
		fatal("Framebuffer %i incomplete: status %x", id, status);

	return *this;
}


static void attach_low (t_fbo& fbo, t_fbo::t_attachment_ptr att,
			GLenum slot, int slice)
{
	if ((fbo.width != 0 && att->width != fbo.width)
	|| (fbo.height != 0 && att->height != fbo.height)) {
		fatal("Tried to attach object with dimensions %i, %i "
			"to FBO with dimensions %i, %i",
			att->width, att->height, fbo.width, fbo.height);
	}
	fbo.width = att->width;
	fbo.height = att->height;

	glBindFramebuffer(GL_FRAMEBUFFER, fbo.id);

	switch (att->target) {
	case tex2d:
		glFramebufferTexture2D(GL_FRAMEBUFFER, slot,
			GL_TEXTURE_2D, att->id, 0);
		break;
	case tex2d_msaa:
		glFramebufferTexture2D(GL_FRAMEBUFFER, slot,
			GL_TEXTURE_2D_MULTISAMPLE, att->id, 0);
		break;
	case tex2d_array:
		glFramebufferTextureLayer(GL_FRAMEBUFFER, slot,
			att->id, 0, slice);
		break;
	case tex2d_array_msaa:
		glFramebufferTexture3D(GL_FRAMEBUFFER, slot,
			GL_TEXTURE_2D_MULTISAMPLE_ARRAY, att->id, 0, slice);
		break;
	case rbo:
	case rbo_msaa:
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, slot,
			GL_RENDERBUFFER, att->id);
		break;
	default:
		fatal("FBO %i, attachment id %i: invalid target enum %i",
			fbo.id, att->id, att->target);
	}
}

t_fbo& t_fbo::attach_color (t_attachment* att, int idx, short slice)
{
	assert(idx >= 0 && idx < num_clr_attachments);

	if (color[idx].taken()) {
		warning("Replacing existing color attachment #%i on FBO %i "
			"(was object %i, now object %i)",
			idx, id, color[idx]->id, att->id);
	}
	color[idx] = { att, slice };
	attach_low(*this, color[idx], GL_COLOR_ATTACHMENT0 + idx, slice);
	return *this;
}

t_fbo& t_fbo::attach_depth (t_attachment* att, short slice)
{
	if (depth.taken()) {
		warning("Replacing existing depth attachment on FBO %i"
			"(was object %i, now object %i)",
			id, depth->id, att->id);
	}
	depth = { att, slice };
	attach_low(*this, depth, GL_DEPTH_ATTACHMENT, slice);
	return *this;
}

void t_fbo::clear_color (int idx)
{
	assert(idx >= 0 && idx < num_clr_attachments);
	color[idx] = { nullptr, 0 };
}

void t_fbo::clear_depth ()
{
	depth = { nullptr, 0 };
}

/*
 * ================= Making different attachments =================
 */

static void generate_tex2d (t_attachment& a)
{
	glGenTextures(1, &a.id);
	glBindTexture(GL_TEXTURE_2D, a.id);

	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, false);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glTextureStorage2D(a.id, 1, a.pixel_type_combined, a.width, a.height);
}

static void generate_tex2d_msaa (t_attachment& a)
{
	glGenTextures(1, &a.id);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, a.id);
	glTextureStorage2DMultisample(a.id, a.samples,
			a.pixel_type_combined, a.width, a.height, true);
}

static void generate_tex2d_array (t_attachment& a)
{
	glGenTextures(1, &a.id);
	glBindTexture(GL_TEXTURE_2D_ARRAY, a.id);

	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_GENERATE_MIPMAP, false);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S,
			GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T,
			GL_CLAMP_TO_EDGE);

	glTextureStorage3D(a.id, 1, a.pixel_type_combined,
			a.width, a.height, a.depth);
}

static void generate_tex2d_array_msaa (t_attachment& a)
{
	glGenTextures(1, &a.id);
	glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, a.id);

	glTextureStorage3DMultisample(a.id, a.samples, a.pixel_type_combined,
			a.width, a.height, a.depth, true);
}

static void generate_rbo (t_attachment& a)
{
	glGenRenderbuffers(1, &a.id);
	glBindRenderbuffer(GL_RENDERBUFFER, a.id);
	glRenderbufferStorage(GL_RENDERBUFFER, a.pixel_type_combined,
			a.width, a.height);
}

static void generate_rbo_msaa (t_attachment& a)
{
	glGenRenderbuffers(1, &a.id);
	glBindRenderbuffer(GL_RENDERBUFFER, a.id);
	glRenderbufferStorageMultisample(GL_RENDERBUFFER, a.samples,
			a.pixel_type_combined, a.width, a.height);
}

inline t_attachment* attachment_finalize (t_attachment* a)
{
	extern std::pair<GLenum, GLenum> dissect_sized_type (GLenum);
	auto cp = dissect_sized_type(a->pixel_type_combined);
	a->pixel_components = cp.first;
	a->pixel_type = cp.second;

	static void (*const gen[num_attachment_targets]) (t_attachment&) =
		{ generate_tex2d,
		  generate_tex2d_msaa,
		  generate_tex2d_array,
		  generate_tex2d_array_msaa,
		  generate_rbo,
		  generate_rbo_msaa };

	gen[a->target](*a);
	return a;
}


t_attachment* make_tex2d (int w, int h, GLenum t)
{
	auto p = new t_attachment;
	p->width = w;
	p->height = h;
	p->pixel_type_combined = t;
	p->target = tex2d;
	return attachment_finalize(p);
}

t_attachment* make_tex2d_msaa (int w, int h, GLenum t, short samples)
{
	auto p = new t_attachment;
	p->width = w;
	p->height = h;
	p->pixel_type_combined = t;
	p->samples = samples;
	p->target = tex2d_msaa;
	return attachment_finalize(p);
}

t_attachment* make_tex2d_array (int w, int h, int d, GLenum t)
{
	auto p = new t_attachment;
	p->width = w;
	p->height = h;
	p->depth = d;
	p->pixel_type_combined = t;
	p->target = tex2d_array;
	return attachment_finalize(p);
}

t_attachment* make_tex2d_array_msaa (int w, int h, int d,
		GLenum t, short samples)
{
	auto p = new t_attachment;
	p->width = w;
	p->height = h;
	p->depth = d;
	p->samples = samples;
	p->pixel_type_combined = t;
	p->target = tex2d_array_msaa;
	return attachment_finalize(p);
}

t_attachment* make_rbo (int w, int h, GLenum t)
{
	auto p = new t_attachment;
	p->width = w;
	p->height = h;
	p->pixel_type_combined = t;
	p->target = rbo;
	return attachment_finalize(p);
}

t_attachment* make_rbo_msaa (int w, int h, GLenum t, short samples)
{
	auto p = new t_attachment;
	p->width = w;
	p->height = h;
	p->pixel_type_combined = t;
	p->target = rbo_msaa;
	return attachment_finalize(p);
}


/* ========================================================= */

/*
 * Map an OpenGL sized type to a component enum and type enum pair,
 * e.g.
 * GL_RGBA32F => { GL_RGBA, GL_FLOAT }
 */

/*
 * GL_R is part of STRQ and not R, RG, RGB, RGBA, so it is incorrect to
 * use that. That would have been inconvenient for the below macros...
 */
#undef GL_R
#define GL_R GL_RED

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

/* ========================================================= */

static std::vector<t_fbo*> ssbuffers;

void sspace_add_buffer (t_fbo& fbo)
{
	ssbuffers.push_back(&fbo);
}

void sspace_resize_buffers (int w, int h)
{
}
