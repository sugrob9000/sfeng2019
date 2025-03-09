#pragma once
#include "material.h"
#include "model.h"
#include <map>
#include <string>

typedef std::map<std::string, Model*> ModelCache;
typedef std::map<std::string, GLuint> TextureCache;
typedef std::map<std::string, Material*> MaterialCache;
typedef std::map<std::string, GLuint> ShaderCache;

const char* const PATH_MODEL = "resource/models/";
const char* const PATH_TEXTURE = "resource/mat/";
const char* const PATH_MATERIAL = "resource/mat/";
const char* const PATH_SHADER = "resource/shader/";

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