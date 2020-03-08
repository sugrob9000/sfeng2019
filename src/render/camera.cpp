#include "render/camera.h"
#include "render/render.h"
#include "input/cmds.h"

void t_camera::apply ()
{
	using namespace glm;

	render_ctx.proj = perspective(radians(fov), aspect, z_near, z_far);
	render_ctx.view = rotate_xyz(radians(ang - vec3(90.0, 0.0, 0.0)));
	render_ctx.view = translate(render_ctx.view, -pos);
	render_ctx.model = mat4(1.0);
}

MOUSEMOVE_ROUTINE (camera)
{
	camera.ang.x += dy;
	camera.ang.z += dx;
}

COMMAND_ROUTINE (cam_dump_pos)
{
	if (ev != PRESS)
		return;
	std::cout << "pos " << camera.pos
		<< "\nang " << camera.ang << std::endl;
}

static constexpr short move_f = 0;
static constexpr short move_b = 1;
static constexpr short move_l = 2;
static constexpr short move_r = 3;
static bool move_flags[4];
t_camera camera;

static bool speedup;
static bool slowdown;
COMMAND_SET_BOOL (cam_accelerate, speedup);
COMMAND_SET_BOOL (cam_decelerate, slowdown);

void upd_camera_pos ()
{
	float speed = 4.0;
	if (speedup)
		speed *= 2.5;
	if (slowdown)
		speed *= 0.4;

	if (camera.ang.x < -90.0)
		camera.ang.x = -90.0;
	if (camera.ang.x > 90.0)
		camera.ang.x = 90.0;

	using glm::radians;
	float sz = sinf(radians(camera.ang.z));
	float sx = sinf(radians(camera.ang.x));
	float cz = cosf(radians(camera.ang.z));

	vec3 delta(0.0);

	if (move_flags[move_f])
		delta += vec3(sz, cz, -sx);
	if (move_flags[move_b])
		delta -= vec3(sz, cz, -sx);
	if (move_flags[move_l])
		delta -= vec3(cz, -sz, 0.0);
	if (move_flags[move_r])
		delta += vec3(cz, -sz, 0.0);

	if (glm::length(delta) > 0.0f)
		camera.pos += glm::normalize(delta) * speed;
}

COMMAND_ROUTINE (cam_move)
{
	if (args.empty())
		return;

	bool f = (ev == PRESS);

	switch (tolower(args[0][0])) {
	case 'f':
		move_flags[move_f] = f;
		break;
	case 'b':
		move_flags[move_b] = f;
		break;
	case 'l':
		move_flags[move_l] = f;
		break;
	case 'r':
		move_flags[move_r] = f;
		break;
	}
}
