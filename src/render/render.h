#ifndef RENDER_H
#define RENDER_H

#include "inc_gl.h"
#include "inc_general.h"

namespace render
{

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

	t_camera (); 
	t_camera (vec3 pos, vec3 ang,
	          float zf, float zn, float fov);

	/*
	 * Transform the current projection matrix
	 * to correspond to the camera
	 */
	void perspective();
	void apply ();
};

extern SDL_Window* window;
extern SDL_GLContext context;
extern t_camera camera;

extern uint8_t camera_move_flags;

bool init (int resx, int resy);

}

#endif // RENDER_H

