#ifndef RENDER_CTX_H
#define RENDER_CTX_H

#include "inc_gl.h"
#include "misc.h"

enum t_render_stage: uint32_t
{
	G_BUFFERS = 0,
	LIGHTING_LSPACE = 1,
	SHADE_FINAL = 2,

	NUM_RENDER_STAGES
};

struct t_render_ctx
{
	t_render_stage stage;

	mat4 proj;
	mat4 view;
	mat4 model;

	/* Set the matrices as their corresponding uniforms */
	void submit_matrices () const;

	/* Same, but without the model matrix */
	void submit_viewproj () const;
};
extern t_render_ctx render_ctx;


struct t_camera
{
	vec3 pos;
	vec3 ang;

	float z_far;
	float z_near;
	float fov;
	float aspect;

	t_camera () { }
	t_camera (vec3 p, vec3 a, float zf, float zn, float fv, float asp)
		: pos(p), ang(a), z_far(zf), z_near(zn),
		  fov(fv), aspect(asp) { }

	void apply ();
};

extern t_camera camera;
void upd_camera_pos ();

#endif // RENDER_CTX_H
