#pragma once
#include "material.h"
#include "model.h"
#include <map>
#include <string>

using ModelCache = std::map<std::string, Model*>;
using TextureCache = std::map<std::string, GLuint>;
using MaterialCache = std::map<std::string, Material*>;
using ShaderCache = std::map<std::string, GLuint>;

inline const char PATH_MODEL[] = "resource/models/";
inline const char PATH_TEXTURE[] = "resource/mat/";
inline const char PATH_MATERIAL[] = "resource/mat/";
inline const char PATH_SHADER[] = "resource/shader/";

Model* get_model(std::string name);
GLuint get_texture(std::string name);
Material* get_material(std::string name);

GLuint get_frag_shader(const std::string& name);
GLuint get_vert_shader(const std::string& name);
GLuint get_shader(const std::string& name, GLenum type);

// Declare these because some initializers may want to
// put special things into cache to avoid edge cases
extern ModelCache cache_mdl;
extern TextureCache cache_tex;
extern MaterialCache cache_mat;
extern ShaderCache cache_shader;