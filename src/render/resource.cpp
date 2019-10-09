#include "resource.h"

namespace render
{

t_cache_mdl cache_mdl;
t_cache_tex cache_tex;
t_cache_mat cache_mat;
t_cache_shader cache_shader;

t_model* get_model (std::string path)
{
	// we're going to have to return something anyway,
	// so it's okay to use std::map::operator[]
	t_model*& ret = cache_mdl[path];

	if (ret != nullptr)
		return ret;

	ret = new t_model;

	path = PATH_MODEL + path + ".obj";
	t_vertices verts;
	if (!verts.load_obj(path))
		core::fatal("Cannot load model %s", path.c_str());
	ret->load(verts);
	return ret;
}

t_texture get_texture (std::string path)
{
	t_texture& ret = cache_tex[path];

	if (ret != 0)
		return ret;

	ret = load_texture(PATH_TEXTURE + path);
	if (!ret)
		core::fatal("Cannot load texture %s", path.c_str());
	return ret;
}

t_shader get_shader (std::string path, GLenum type)
{
	t_shader& ret = cache_shader[path];

	if (ret != 0) {
		int actual;
		glGetShaderiv(ret, GL_SHADER_TYPE, &actual);
		if (type != actual) {
			core::fatal(
					"Shader %s of the wrong type:\n"
					"actual %i, requested %i",
				path.c_str(), actual, type);
		}
		return ret;
	}

	ret = compile_glsl(PATH_SHADER + path, type);
	if (!ret)
		core::fatal("Cannot load shader %s", path.c_str());
	return ret;
}

}
