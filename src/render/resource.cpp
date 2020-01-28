#include "resource.h"

t_cache_mdl cache_mdl;
t_cache_tex cache_tex;
t_cache_mat cache_mat;
t_cache_shader cache_shader;

/*
 * The get_*() functions have to return something
 * (or crash), so it is okay for them to use
 * std::map::operator[]
 */

t_model* get_model (std::string path)
{
	t_model*& ret = cache_mdl[path];

	if (ret != nullptr)
		return ret;

	ret = new t_model;

	path = PATH_MODEL + path + ".rvd";
	t_model_mem verts;
	verts.load_rvd(path);
	ret->load(verts);
	return ret;
}

GLuint get_texture (std::string path)
{
	GLuint& ret = cache_tex[path];

	if (ret != 0)
		return ret;

	path = PATH_TEXTURE + path;
	ret = load_texture(path);
	if (!ret)
		fatal("Cannot load texture %s", path.c_str());
	return ret;
}

GLuint get_shader (const std::string& path, GLenum type)
{
	GLuint& ret = cache_shader[path];

	if (ret != 0) {
		// shader exists. verify that it is of the right type
		int actual_shader_type;
		glGetShaderiv(ret, GL_SHADER_TYPE, &actual_shader_type);
		if (type != actual_shader_type) {
			fatal("Shader %s of the wrong type:\n"
				"actual %i, requested %i",
				path.c_str(), actual_shader_type, type);
		}
		return ret;
	}

	ret = compile_glsl(path, type);

	if (!ret)
		fatal("Cannot load shader %s", path.c_str());
	return ret;
}

GLuint get_vert_shader (const std::string& name)
{
	return get_shader(PATH_SHADER + name + ".vert", GL_VERTEX_SHADER);
}

GLuint get_frag_shader (const std::string& name)
{
	return get_shader(PATH_SHADER + name + ".frag", GL_FRAGMENT_SHADER);
}

t_material* get_material (std::string path)
{
	t_material*& ret = cache_mat[path];

	if (ret != nullptr)
		return ret;

	path = PATH_MATERIAL + path;
	ret = new t_material;
	ret->load(path);
	return ret;
}
