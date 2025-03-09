#include "resource.h"

ModelCache cache_mdl;
TextureCache cache_tex;
MaterialCache cache_mat;
ShaderCache cache_shader;

// The get_*() functions have to return something
// (or crash), so it is okay for them to use
// std::map::operator[]

Model* get_model(std::string path) {
  Model*& ret = cache_mdl[path];

  if (ret != nullptr)
    return ret;

  ret = new Model;

  path = PATH_MODEL + path + ".rvd";
  InMemoryModel verts;
  verts.load_rvd(path);
  ret->load(verts);
  return ret;
}

GLuint get_texture(std::string path) {
  GLuint& ret = cache_tex[path];

  if (ret != 0)
    return ret;

  path = PATH_TEXTURE + path;
  ret = load_texture(path);
  if (!ret)
    fatal("Cannot load texture %s", path.c_str());
  return ret;
}

GLuint get_shader(const std::string& path, GLenum type) {
  GLuint& ret = cache_shader[path];

  if (ret != 0) {
    // shader exists. verify that it is of the right type
    int actual_shader_type;
    glGetShaderiv(ret, GL_SHADER_TYPE, &actual_shader_type);
    if (type != actual_shader_type) {
      fatal(
        "Shader %s of the wrong type:\n"
        "actual %i, requested %i",
        path.c_str(),
        actual_shader_type,
        type
      );
    }
    return ret;
  }

  ret = compile_glsl(PATH_SHADER + path, type);

  if (!ret)
    fatal("Cannot load shader %s", path.c_str());
  return ret;
}

GLuint get_vert_shader(const std::string& name) {
  return get_shader(name + ".vert", GL_VERTEX_SHADER);
}

GLuint get_frag_shader(const std::string& name) {
  return get_shader(name + ".frag", GL_FRAGMENT_SHADER);
}

Material* get_material(std::string path) {
  Material*& ret = cache_mat[path];

  if (ret != nullptr)
    return ret;

  path = PATH_MATERIAL + path;
  ret = new Material;
  ret->load(path);
  return ret;
}
