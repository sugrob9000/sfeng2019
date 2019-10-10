#include "resource.h"

namespace render
{

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

	path = PATH_MODEL + path + ".obj";
	t_model_mem verts;
	if (!verts.load_obj(path))
		core::fatal("Cannot load model %s", path.c_str());
	ret->load(verts);
	return ret;
}

t_texture_id get_texture (std::string path)
{
	t_texture_id& ret = cache_tex[path];

	if (ret != 0)
		return ret;

	path = PATH_TEXTURE + path;
	ret = load_texture(path);
	if (!ret)
		core::fatal("Cannot load texture %s", path.c_str());
	return ret;
}

t_shader_id get_shader (std::string path, GLenum type)
{
	t_shader_id& ret = cache_shader[path];

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

	path = PATH_SHADER + path + ".glsl";
	ret = compile_glsl(path, type);
	if (!ret)
		core::fatal("Cannot load shader %s", path.c_str());
	return ret;
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

}
