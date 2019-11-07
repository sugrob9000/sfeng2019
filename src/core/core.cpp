#include "core.h"
#include "entity.h"
#include "inc_general.h"
#include "input/cmds.h"
#include "input/input.h"
#include "render/render.h"

namespace core
{

bool must_quit;
int exit_code;
long long tick;

void init (std::string conf)
{
	must_quit = false;
	exit_code = 0;
	tick = 0;

	fill_ent_registry();

	input::run_script(conf);
}

void update ()
{
	tick++;

	for (e_base* e: ents.vec)
		e->think();

	while (!signals.empty()) {
		const t_signal& s = signals.top();
		if (s.tick_due > tick) {
			// all signals after this one
			// are for later, too
			break;
		}
		s.execute();
		signals.pop();
	}

	render::upd_camera_pos();
}

void load_map (std::string path)
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
			kv.clear();
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

} // namespace core

COMMAND_ROUTINE (nop)
{
	return;
}

COMMAND_ROUTINE (exit)
{
	if (ev != PRESS)
		return;
	core::exit_code = 0;
	if (!args.empty())
		core::exit_code = std::atoi(args[0].c_str());
	core::must_quit = true;
}

COMMAND_ROUTINE (echo)
{
	if (ev != PRESS)
		return;

	for (const std::string& arg: args)
		std::cout << arg << " ";

	std::cout << std::endl;
}
