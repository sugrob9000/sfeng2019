#include "inc_general.h"
#include "core.h"
#include "render/render.h"
#include "ent/entity.h"

namespace core
{

t_game game;

void init ()
{
	game.must_quit = false;
	game.exit_code = 0;
	game.tick = 0;
}

void t_game::update ()
{
	tick++;

	for (e_base* e: ents.v)
		e->think();

	const float cam_speed = 1.0;
	vec3 cam_delta;
	auto& flags = render::cam_move_flags;
	render::t_camera& cam = render::camera;

	if (cam.ang.x < -90.0)
		cam.ang.x = -90.0;
	if (cam.ang.x > 90.0)
		cam.ang.x = 90.0;

	float sz = sinf(cam.ang.z * DEG_TO_RAD);
	float sx = sinf(cam.ang.x * DEG_TO_RAD);
	float cz = cosf(cam.ang.z * DEG_TO_RAD);

	if (flags[render::cam_move_f])
		cam_delta += vec3(sz, cz, -sx);
	if (flags[render::cam_move_b])
		cam_delta -= vec3(sz, cz, -sx);
	if (flags[render::cam_move_l])
		cam_delta -= vec3(cz, -sz, 0.0);
	if (flags[render::cam_move_r])
		cam_delta += vec3(cz, -sz, 0.0);

	cam_delta.norm();
	cam.pos += cam_delta * cam_speed;
}

void t_game::load_map (std::string path)
{
	std::ifstream f(path);

	if (!f)
		fatal("Could not open world file %s", path.c_str());

	e_base* cur_ent = nullptr;
	t_ent_keyvals kv;

	auto finalize = [&] ()
		{
			if (cur_ent != nullptr)
				cur_ent->apply_keyvals(kv);
		};

	for (std::string line; std::getline(f, line); ) {
		int comment = line.find('#');
		if (comment != std::string::npos)
			line.erase(comment, std::string::npos);

		int ws_end = line.length()-1;
		while (ws_end >= 0 && isspace(line[ws_end]))
			ws_end--;
		line.erase(ws_end+1, std::string::npos);

		if (line.empty())
			continue;

		if (isspace(line[0])) {
			// begins with whitespace: is a key-value

			int key_begin = 0;
			int n = line.length();
			while (key_begin < n && isspace(line[key_begin]))
				key_begin++;

			int key_end = key_begin;
			while (key_end < n && !isspace(line[key_end]))
				key_end++;

			kv.add(line.substr(key_begin, key_end-key_begin),
			       line.substr(key_end + 1, std::string::npos));
		} else {
			// does not begin with whitespace: is a new entity

			finalize();
			cur_ent = ents.spawn(line);
			if (cur_ent == nullptr) {
				core::fatal(
					"Map %s: cannot spawn entity \"%s\"",
					path.c_str(), line.c_str());
			}
		}
	}
	finalize();
}

}
