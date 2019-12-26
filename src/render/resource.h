#ifndef RESOURCE_H
#define RESOURCE_H

#include "material.h"
#include "model.h"
#include <map>

typedef std::map<std::string, t_model*> t_cache_mdl;
typedef std::map<std::string, t_texture_id> t_cache_tex;
typedef std::map<std::string, t_material*> t_cache_mat;
typedef std::map<std::string, t_shader_id> t_cache_shader;

const char* const PATH_MODEL = "res/models/";
const char* const PATH_TEXTURE = "res/mat/";
const char* const PATH_MATERIAL = "res/mat/";
const char* const PATH_SHADER = "res/shader/";

t_model* get_model (std::string name);
t_texture_id get_texture (std::string name);
t_material* get_material (std::string name);

t_shader_id get_frag_shader (const std::string& name);
t_shader_id get_vert_shader (const std::string& name);

/*
 * Declare these because some initializers may want to
 * put special things into cache to avoid edge cases
 */
extern t_cache_mdl cache_mdl;
extern t_cache_tex cache_tex;
extern t_cache_mat cache_mat;
extern t_cache_shader cache_shader;

#endif // RESOURCE_H
