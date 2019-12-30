#include "ent/lights.h"
#include "inc_general.h"
#include "inc_gl.h"
#include "input/cmds.h"
#include "material.h"
#include "render.h"
#include "resource.h"
#include "shaderlib.h"
#include <cassert>

t_shader_id compile_glsl (std::string path, GLenum type)
{
	std::ifstream f(path);

	if (!f)
		return 0;

	int filesize;
	f.seekg(0, f.end);
	filesize = f.tellg();
	f.seekg(0, f.beg);

	std::string source;
	source.reserve(filesize);

	for (std::string line; std::getline(f, line); ) {
		source += line;
		source += '\n';
	}

	unsigned int id = glCreateShader(type);

	const char* const source_ptr = source.c_str();

	// to glShaderSource, pretend the source is just one line
	// which does not affect the syntax because we put linebreaks
	glShaderSource(id, 1, &source_ptr, nullptr);
	glCompileShader(id);

	int success = 0;
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);

	if (success)
		return id;

	int log_length = 0;
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_length);
	char log[log_length+1];
	log[log_length] = 0;
	glGetShaderInfoLog(id, log_length, 0, log);

	warning("Failed to compile shader %s:\n%s\n",
			path.c_str(), log);
	return 0;
}

t_material mat_none_instance;
t_material* mat_none = &mat_none_instance;

t_material mat_occlude_instance;
t_material* mat_occlude = &mat_occlude_instance;

void init_materials ()
{
	cache_mat[""] = mat_none;
	cache_mat["OCCLUDE"] = mat_occlude;
}

void t_material::load (const std::string& path)
{
	std::ifstream f(path);

	if (!f)
		fatal("Material %s: cannot open file", path.c_str());

	name = path;

	std::string key;
	std::string value;

	struct bitmap_desc_interm {
		std::string loc_name;
		t_texture_id texid;
	};

	std::vector<bitmap_desc_interm> bitmaps_interm;

	std::string vert_name;
	std::string frag_name;

	while (true) {
		f >> key >> value;
		f.ignore(-1, '\n');
		if (!f)
			break;

		if (key == "FRAG") {
			frag_name = value;
			continue;
		} else if (key == "VERT") {
			vert_name = value;
			continue;
		}

		key = "map_" + key;
		bitmaps_interm.push_back({ key, get_texture(value) });
	}

	frag = get_frag_shader(frag_name);
	vert = get_vert_shader(vert_name);

	program = glCreateProgram();
	glAttachShader(program, get_frag_shader("common/lib"));
	glAttachShader(program, get_vert_shader("common/lib"));
	glAttachShader(program, frag);
	glAttachShader(program, vert);
	glLinkProgram(program);

	int link_success = 0;
	glGetProgramiv(program, GL_LINK_STATUS, &link_success);
	if (!link_success) {
		int log_length = 0;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &log_length);
		char log[log_length+1];
		log[log_length] = 0;
		glGetProgramInfoLog(program, log_length, &log_length, log);
		fatal("OpenGL shader program for material %s "
				"failed to link:\n%s", path.c_str(), log);
	}

	for (const bitmap_desc_interm& d: bitmaps_interm) {
		int location = glGetUniformLocation(program,
				d.loc_name.c_str());
		if (location != -1) {
			bitmaps.push_back({ location, d.texid });
		} else {
			warning("%s is not a valid uniform in material %s",
				d.loc_name.c_str(), path.c_str());
		}
	}

	glBindAttribLocation(program, ATTRIB_LOC_TANGENT, "tangent");
}

/*
 * Material application is idempotent, so we can avoid redundancy
 */
const t_material* latest_material = nullptr;
t_render_stage latest_render_stage;

void material_barrier ()
{
	latest_material = nullptr;
}

void t_material::apply (t_render_stage s) const
{
	if (latest_material == this && s == latest_render_stage)
		return;
	latest_material = this;
	latest_render_stage = s;

	glUseProgram(program);
	for (int i = 0; i < bitmaps.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i + 2);
		glBindTexture(GL_TEXTURE_2D, bitmaps[i].texid);
		glUniform1i(bitmaps[i].location, i + 2);
	}

	glUniform1ui(UNIFORM_LOC_RENDER_STAGE, s);

	light_apply_uniforms(s);
}

int get_surface_gl_format (SDL_Surface* s)
{
	// checking where alpha and red channels are
	// should be enough to understand the format
	switch (((uint64_t) s->format->Amask << 32)
		| (uint64_t) s->format->Rmask) {
	case 0x00FF0000:
		return GL_BGR;
	case 0x000000FF:
		return GL_RGB;
	case 0xFF00000000FF0000:
		return GL_BGRA;
	case 0xFF000000000000FF:
		return GL_RGBA;
	default:
		return -1;
	}
}

t_texture_id load_texture (std::string path)
{
	SDL_Surface* surf = IMG_Load(path.c_str());

	if (surf == nullptr)
		return 0;

	int format = get_surface_gl_format(surf);
	if (format == -1) {
		warning("Texture %s uses bogus format", path.c_str());
		SDL_FreeSurface(surf);
		return 0;
	}

	t_texture_id id;

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);

	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h,
			0, format, GL_UNSIGNED_BYTE, surf->pixels);

	SDL_FreeSurface(surf);

	return id;
}
