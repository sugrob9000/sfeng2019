#ifndef CAMERA_H
#define CAMERA_H

#include "inc_gl.h"
#include "misc.h"

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

#endif // CAMERA_H
