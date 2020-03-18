#include "render/ssbuffer.h"
#include <vector>
#include <set>

static std::vector<t_fbo*> ssbuffers;

void add_sspace_buffer (t_fbo* ptr)
{
	ssbuffers.push_back(ptr);
}

void resize_sspace_buffers (int w, int h)
{
	std::set<GLuint> already_updated;

	auto try_update = [&] (t_attachment& att)
	-> void {
		if (already_updated.count(att.id))
			return;
		att.update(w, h, att.depth, att.samples);
		already_updated.insert(att.id);
	};

	for (t_fbo* it: ssbuffers) {
		t_fbo& fbo = *it;

		for (int i = 0; i < t_fbo::NUM_COLOR_ATTACHMENTS; i++)
			try_update(fbo.color[i]);

		try_update(fbo.depth);
	}
}
