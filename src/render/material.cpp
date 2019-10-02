#include "render.h"
#include "material.h"
#include "inc_general.h"
#include "inc_gl.h"

namespace render
{

unsigned int compile_glsl (std::string path, GLenum type)
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

t_material::t_material (std::string mtfpath)
{
	load_mtf(mtfpath);
}

bool t_material::load_mtf (std::string path)
{
	return false;
}

void t_material::apply ()
{
	// TODO: apply shaders, etc.
}


t_texture::t_texture ()
{
}

t_texture::t_texture (std::string path)
{
	if (!load(path)) {
		core::warning("Failed to load texture %s",
				path.c_str());
	}
}

bool t_texture::load (std::string path)
{
	SDL_Surface* surf = IMG_Load(path.c_str());

	if (surf == nullptr)
		return false;

	GLuint texid;

	glGenTextures(1, &texid);
	glBindTexture(GL_TEXTURE_2D, texid);

	glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);

	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);

	// mipmap
	int level = 0;
	int format = GL_BGR;

	int w = surf->w;
	int h = surf->h;
	GLubyte* p = (GLubyte*) surf->pixels;

	while (h >= 1 && w >= 1) {
		glTexImage2D(GL_TEXTURE_2D, level++,
				GL_COMPRESSED_RGBA, w, h,
				0, format, GL_UNSIGNED_BYTE, p);
		gluScaleImage(format,
				w, h, GL_UNSIGNED_BYTE, p,
				w / 2, h / 2, GL_UNSIGNED_BYTE, p);
		w /= 2;
		h /= 2;
	}

	SDL_FreeSurface(surf);

	id = texid;
	return true;
}

}
