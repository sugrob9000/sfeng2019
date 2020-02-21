#include "ent/reflector.h"
#include "misc.h"
#include "inc_gl.h"
#include "render/render.h"
#include "render/vis.h"
#include "render/framebuffer.h"
#include "render/material.h"

std::vector<e_reflector*> reflectors;

FILL_IO_DATA (reflector)
{
	BASIC_SIG_HANDLERS(reflector);
}

void lt(std::string path, int i)
{
	SDL_Surface* surf = IMG_Load(path.c_str());

	if (surf == nullptr)
		return;

	int format = get_surface_gl_format(surf);
	if (format == -1) {
		warning("Texture %s uses bogus format", path.c_str());
		SDL_FreeSurface(surf);
		return;
	}

	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, surf->w, surf->h,
			0, format, GL_UNSIGNED_BYTE, surf->pixels);

	SDL_FreeSurface(surf);
}

void e_reflector::post_init ()
{
	glGenFramebuffers(1, &ref_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, ref_fbo);

	glGenTextures(1, &cubemap);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);

	GLenum t = GL_TEXTURE_CUBE_MAP;		
	glTexParameteri(t, GL_GENERATE_MIPMAP, GL_FALSE);
	glTexParameteri(t, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(t, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(t, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(t, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(t, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	for (int i = 0; i < 6; i++) {
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 
				0, GL_RGB, resolution, resolution,
				0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i,
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, cubemap, 0);	
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	GLuint rbo;
	glGenRenderbuffers(1, &rbo);
	glBindRenderbuffer(GL_RENDERBUFFER, rbo);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 
			resolution, resolution);

	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, 
			GL_RENDERBUFFER, rbo);

	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// lt("d:/cubemap/cm_pos_x.png", 4);
	// lt("d:/cubemap/cm_neg_x.png", 5);
	// lt("d:/cubemap/cm_pos_y.png", 0);
	// lt("d:/cubemap/cm_neg_y.png", 1);
	// lt("d:/cubemap/cm_pos_z.png", 2);
	// lt("d:/cubemap/cm_neg_z.png", 3);

	// glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	reflectors.push_back(this);
}

void e_reflector::render (t_render_stage s) const {}
void e_reflector::think () { pos.x -= 50.0 * sin(tick * 0.1); }
t_bound_box e_reflector::get_bbox () const {return {};}

void e_reflector::apply_keyvals (const t_ent_keyvals& kv)
{
	apply_basic_keyvals(kv);

	KV_TRY_GET(kv["resolution"],
			resolution = atoi(val.c_str());,
			resolution = 128; );

	post_init();
}

vec3 angs[6] = {
	{0, -90, 90},
	{0, 90, -90},
	{0, 0, 0},
	{0, 180, 180},
	{-90, 0, 0},
	{90, 0, 180}
};

void e_reflector::render_cubemap ()
{
	push_reset_matrices();
	
	glMatrixMode(MTX_MODEL);
	glLoadIdentity();

	glMatrixMode(MTX_VIEWPROJ);
	glLoadIdentity();
	gluPerspective(90, 1, 0.1, 1000);
	glRotatef(-90.0, 1.0, 0.0, 0.0);

	glViewport(0, 0, resolution, resolution);

	glBindFramebuffer(GL_FRAMEBUFFER, ref_fbo);
	for (int i = 0; i < 6; i++) {
		glDrawBuffer(GL_COLOR_ATTACHMENT0 + i);
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		glPushMatrix();

		rotate_gl_matrix(angs[i]);
		translate_gl_matrix(-pos);

		draw_sky();
		all_leaves.render(REF_RENDER);

		glPopMatrix();
	}
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	pop_matrices();
}

void compute_reflections ()
{
	for (e_reflector* r : reflectors) {
		r->render_cubemap();
	}
}