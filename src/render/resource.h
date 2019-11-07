#ifndef RESOURCE_H
#define RESOURCE_H

#include "material.h"
#include "model.h"
#include <map>

namespace render
{

typedef std::map<std::string, t_model*> t_cache_mdl;
typedef std::map<std::string, t_texture_id> t_cache_tex;
typedef std::map<std::string, t_material*> t_cache_mat;
typedef std::map<std::string, t_shader_id> t_cache_shader;

const char* const PATH_MODEL = "res/models/";
const char* const PATH_TEXTURE = "res/textures/";
const char* const PATH_MATERIAL = "res/materials/";
const char* const PATH_SHADER = "res/shaders/";

t_model* get_model (std::string path);
t_texture_id get_texture (std::string path);
t_material* get_material (std::string path);
t_shader_id get_shader (std::string path, GLenum type);

};

#endif // RESOURCE_H
