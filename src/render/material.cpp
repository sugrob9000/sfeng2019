#include "render.h"
#include "material.h"
#include "inc_general.h"
#include "inc_gl.h"
#include "resource.h"
#include "input/cmds.h"

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

t_material mat_none;
void init_materials ()
{
	mat_none.program = 0;
	cache_mat[""] = &mat_none;
}

void t_material::load (std::string path)
{
	std::ifstream f(path);

	if (!f)
		fatal("Material %s: cannot open file", path.c_str());

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

	frag = get_shader(frag_name, GL_FRAGMENT_SHADER);
	vert = get_shader(vert_name, GL_VERTEX_SHADER);

	GLuint frag_lib = get_shader("common/frag_lib", GL_FRAGMENT_SHADER);

	program = glCreateProgram();
	glAttachShader(program, frag_lib);
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

	light_pass_flag_loc = glGetUniformLocation(program, "light_pass");

	for (const bitmap_desc_interm& d: bitmaps_interm) {
		int location = glGetUniformLocation(program,
				d.loc_name.c_str());
		if (location != -1) {
			bitmaps.push_back({ location, d.texid });
		} else {
			warning("Material %s: %s is not a valid "
				"uniform in shaders %s, %s",
				path.c_str(), d.loc_name.c_str(),
				frag_name.c_str(), vert_name.c_str());
		}
	}
}

void t_material::apply (bool light_pass)
{
	glUseProgram(program);
	for (int i = 0; i < bitmaps.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, bitmaps[i].texid);
		glUniform1i(bitmaps[i].location, i);
	}
	glUniform1i(light_pass_flag_loc, light_pass);
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
