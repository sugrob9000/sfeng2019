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
		const char* hint = (width == 0 && height == 0)
			? " - perhaps nothing has been attached?" : "";
		fatal("Framebuffer %i with invalid dimensions %i, %i%s",
				id, width, height, hint);
	}

	if (status != GL_FRAMEBUFFER_COMPLETE)
		fatal("Framebuffer %i incomplete: status %x", id, status);

	return *this;
}


static void attach_low (t_fbo& fbo, t_fbo::t_attachment_ptr att,
			GLenum slot, int slice)
{
	if (fbo.width == 0 && fbo.height == 0) {
		fbo.width = att->width;
		fbo.height = att->height;
	} else if (fbo.width != att->width || fbo.height != att->height) {
		fatal("Tried to attach object %i with dimensions %i, %i "
			"to framebuffer %i with dimensions %i, %i",
			att->id, att->width, att->height,
			fbo.id, fbo.width, fbo.height);
	}

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

t_fbo& t_fbo::set_mrt_slots (const std::vector<GLenum>& slots)
{
	bind();
	glDrawBuffers(slots.size(), slots.data());
	return *this;
}

/*
 * ================= Making different attachments =================
 */

inline t_attachment* attachment_finalize (t_attachment* a)
{
	GLenum t;
	switch (a->target) {
	case tex2d:
		t = GL_TEXTURE_2D;
		glGenTextures(1, &a->id);
		glBindTexture(t, a->id);
		glTexParameteri(t, GL_GENERATE_MIPMAP, false);
		glTexParameteri(t, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(t, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(t, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(t, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureStorage2D(a->id, 1, a->storage_type,
				a->width, a->height);
		break;
	case tex2d_msaa:
		t = GL_TEXTURE_2D_MULTISAMPLE;
		glGenTextures(1, &a->id);
		glBindTexture(t, a->id);
		glTextureStorage2DMultisample(a->id, a->samples,
				a->storage_type, a->width, a->height, true);
		break;
	case tex2d_array:
		t = GL_TEXTURE_2D_ARRAY;
		glGenTextures(1, &a->id);
		glBindTexture(t, a->id);
		glTexParameteri(t, GL_GENERATE_MIPMAP, false);
		glTexParameteri(t, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(t, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(t, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(t, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTextureStorage3D(a->id, 1, a->storage_type,
				a->width, a->height, a->depth);
		break;
	case tex2d_array_msaa:
		glGenTextures(1, &a->id);
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, a->id);
		glTextureStorage3DMultisample(a->id, a->samples,
				a->storage_type,
				a->width, a->height, a->depth, true);
		break;
	case rbo:
		glGenRenderbuffers(1, &a->id);
		glBindRenderbuffer(GL_RENDERBUFFER, a->id);
		glRenderbufferStorage(GL_RENDERBUFFER,
				a->storage_type, a->width, a->height);
		break;
	case rbo_msaa:
		glGenRenderbuffers(1, &a->id);
		glBindRenderbuffer(GL_RENDERBUFFER, a->id);
		glRenderbufferStorageMultisample(GL_RENDERBUFFER, a->samples,
				a->storage_type, a->width, a->height);
		break;
	}

	return a;
}


t_attachment* make_tex2d (int w, int h, GLenum t)
{
	auto p = new t_attachment;
	p->width = w;
	p->height = h;
	p->storage_type = t;
	p->target = tex2d;
	return attachment_finalize(p);
}

t_attachment* make_tex2d_msaa (int w, int h, GLenum t, short samples)
{
	auto p = new t_attachment;
	p->width = w;
	p->height = h;
	p->storage_type = t;
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
	p->storage_type = t;
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
	p->storage_type = t;
	p->target = tex2d_array_msaa;
	return attachment_finalize(p);
}

t_attachment* make_rbo (int w, int h, GLenum t)
{
	auto p = new t_attachment;
	p->width = w;
	p->height = h;
	p->storage_type = t;
	p->target = rbo;
	return attachment_finalize(p);
}

t_attachment* make_rbo_msaa (int w, int h, GLenum t, short samples)
{
	auto p = new t_attachment;
	p->width = w;
	p->height = h;
	p->storage_type = t;
	p->target = rbo_msaa;
	return attachment_finalize(p);
}

/* ========================================================= */

static std::vector<t_fbo*> ssbuffers;

void sspace_add_buffer (t_fbo& fbo)
{
	ssbuffers.push_back(&fbo);
}

void sspace_resize_buffers (int w, int h)
{
	std::set<t_attachment*> updated;

	auto upd = [&] (t_fbo::t_attachment_ptr& a) -> void {
		if (updated.count(a.ptr))
			return;
		updated.insert(a.ptr);

		switch (a->target) {
		case tex2d:
		case tex2d_msaa:
		case tex2d_array:
		case tex2d_array_msaa:
			glDeleteTextures(1, &a->id);
			break;
		case rbo:
		case rbo_msaa:
			glDeleteRenderbuffers(1, &a->id);
			break;
		}

		// leave the rest of the parameters as were
		a->width = w;
		a->height = h;

		// regenerate the actual texture that has just been
		// deleted and repopulated with the new parameters
		attachment_finalize(a.ptr);
	};

	for (t_fbo* fbo: ssbuffers) {
		fbo->width = w;
		fbo->height = h;

		for (int i = 0; i < t_fbo::num_clr_attachments; i++) {
			t_fbo::t_attachment_ptr p = fbo->color[i];
			if (!p.taken())
				continue;

			upd(p);
			fbo->clear_color(i);
			fbo->attach_color(p.ptr, i, p.slice_used);
		}

		t_fbo::t_attachment_ptr p = fbo->depth;
		if (!p.taken())
			continue;

		upd(p);
		fbo->clear_depth();
		fbo->attach_depth(p.ptr, p.slice_used);
	}
}
