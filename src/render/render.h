#ifndef RENDER_H
#define RENDER_H

#include "inc_gl.h"

namespace render
{

class t_render_info
{
	public:

	void render (vec3 pos);
};

class t_camera
{
	public:

	vec3 pos;
	vec3 ang;

	float z_far;
	float z_near;
	float fov;

	/*
	 * Cannot specify the camera without
	 * the essentials such as fov and z-planes
	 */
	t_camera (vec3, vec3) = delete;
	t_camera (vec3) = delete;
	t_camera () = delete;

	t_camera (vec3 pos, vec3 ang,
	          float zf, float zn, float fov);

	/*
	 * Transform the current projection matrix
	 * to correspond to the camera
	 */
	void apply ();
};

extern SDL_Window* window;
extern SDL_GLContext context;

bool init (int resx, int resy);

}

#endif // RENDER_H

