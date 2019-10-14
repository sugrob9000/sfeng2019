#include "render.h"
#include "material.h"
#include "inc_general.h"
#include "inc_gl.h"
#include "resource.h"

namespace render
{

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

	const char* source_ptr = source.c_str();

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

	char* log = new char[log_length];
	glGetShaderInfoLog(id, log_length, 0, log);

	core::warning("Failed to compile shader %s:\n%s\n",
			path.c_str(), log);

	delete[] log;

	return 0;
}

void t_material::load(std::string path)
{
	std::ifstream f(path);

	if (!f)
		core::fatal("Material %s: cannot open file", path.c_str());

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

	t_shader_id frag = get_shader(frag_name, GL_FRAGMENT_SHADER);
	t_shader_id vert = get_shader(vert_name, GL_VERTEX_SHADER);

	program = glCreateProgram();
	glAttachShader(program, frag);
	glAttachShader(program, vert);
	glLinkProgram(program);

	glUseProgram(program);
	for (const bitmap_desc_interm& d: bitmaps_interm) {
		int location = glGetUniformLocation(program,
				d.loc_name.c_str());
		if (location != -1) {
			bitmaps.push_back({ location, d.texid });
		} else {
			core::warning("Material %s:\n"
				"%s is not a valid uniform in shaders %s, %s",
				path.c_str(), d.loc_name.c_str(),
				frag_name.c_str(), vert_name.c_str());
		}
	}
}

void t_material::apply ()
{
	glUseProgram(program);
	for (int i = 0; i < bitmaps.size(); i++) {
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, bitmaps[i].texid);
		glUniform1i(bitmaps[i].location, i);
	}
}


t_texture_id load_texture (std::string path)
{
	SDL_Surface* surf = IMG_Load(path.c_str());

	if (surf == nullptr)
		return 0;

	t_texture_id id;

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);

	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);

	// mipmap
	int level = 0;
	int format;

	// checking where alpha and red channels are
	// should be enough to understand the format
	switch ((uint64_t) surf->format->Rmask
		| ((uint64_t) surf->format->Amask << 32)) {
	// without alpha
	case 0x00FF0000:
		format = GL_BGR;
		break;
	case 0x000000FF:
		format = GL_RGB;
		break;
	// with alpha
	case 0xFF00000000FF0000:
		format = GL_BGRA;
		break;
	case 0xFF000000000000FF:
		format = GL_RGBA;
		break;
	default:
		core::warning("Texture %s uses bogus format", path.c_str());
		return 0;
	}

	int w = surf->w;
	int h = surf->h;
	GLubyte* p = (GLubyte*) surf->pixels;

	while (h >= 1 && w >= 1) {
		glTexImage2D(GL_TEXTURE_2D, level++,
				GL_RGBA, w, h,
				0, format, GL_UNSIGNED_BYTE, p);
		gluScaleImage(format,
				w, h, GL_UNSIGNED_BYTE, p,
				w / 2, h / 2, GL_UNSIGNED_BYTE, p);
		w /= 2;
		h /= 2;
	}

	SDL_FreeSurface(surf);

	return id;
}

}
