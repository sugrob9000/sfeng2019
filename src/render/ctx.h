#ifndef RENDER_CTX_H
#define RENDER_CTX_H

#include "inc_gl.h"
#include "misc.h"
#include <array>

enum t_render_stage: uint32_t
{
	RENDER_STAGE_G_BUFFERS = 0,
	RENDER_STAGE_LIGHTING_LSPACE = 1,
	RENDER_STAGE_SHADE_FINAL = 2,

	RENDER_STAGE_WIREFRAME = 3,

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


/*
 * Use RAII to save and restore matrices, e.g.:
 * {
 *	matrix_restorer r(render_ctx);
 *	my_camera.apply(); // changes matrices
 *	// ... do some rendering
 * }
 * after exiting the scope, matrices are restored automatrically
 */
struct matrix_restorer
{
	t_render_ctx* ctx;
	mat4 proj;
	mat4 view;
	mat4 model;
	matrix_restorer (t_render_ctx& c)
		: ctx(&c), proj(c.proj), view(c.view), model(c.model) { }
	~matrix_restorer () {
		ctx->proj = proj;
		ctx->view = view;
		ctx->model = model;
	}
};


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

	mat4 get_view ();
	mat4 get_proj ();

	/*
	 * The four points in worldspace that correspond to this
	 * camera's screen corners at a given depth
	 */
	std::array<vec3, 4> corner_points (float depth);
};

extern t_camera camera;
void upd_camera_pos ();

#endif // RENDER_CTX_H
