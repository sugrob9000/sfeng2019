#include "inc_general.h"
#include "core.h"
#include "render/render.h"
#include "ent/entity.h"

namespace core
{

t_game game;

void init ()
{
}

t_game::t_game ()
{
	must_quit = false;
	exit_code = 0;
	tick = 0;
}

void t_game::update ()
{
	tick++;

	for (e_base* e: ents.v)
		e->think();

	const float cam_speed = 1.0;
	vec3 cam_delta;
	uint8_t& flags = render::camera_move_flags;
	vec3& ang = render::camera.ang;
	if (flags & 1) {
		cam_delta += vec3(
				-sinf(ang.y * M_PI / 180.0),
				sinf(ang.x * M_PI / 180.0),
				cosf(ang.y * M_PI / 180.0));
	}
	if (flags & 2) {
		cam_delta -= vec3(
				-sinf(ang.y * M_PI / 180.0),
				sinf(ang.x * M_PI / 180.0),
				cosf(ang.y * M_PI / 180.0));
	}
	if (flags & 4) {
		cam_delta += vec3(
				cosf(ang.y * M_PI / 180.0), 0.0,
				sinf(ang.y * M_PI / 180.0));
	}
	if (flags & 8) {
		cam_delta -= vec3(
				cosf(ang.y * M_PI / 180.0), 0.0,
				sinf(ang.y * M_PI / 180.0));
	}
	cam_delta *= cam_speed;
	render::camera.pos += cam_delta;

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
			       line.substr(key_end + 1));
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
