#include "ent/lights.h"
#include "inc_gl.h"
#include "input/cmds.h"
#include "render/material.h"
#include "render/render.h"
#include "render/resource.h"
#include <cassert>
#include <algorithm>

t_material mat_none_instance;
t_material* mat_none = &mat_none_instance;

t_material mat_occlude_instance;
t_material* mat_occlude = &mat_occlude_instance;

void init_materials ()
{
	cache_mat[""] = mat_none;
	cache_mat["None"] = mat_none;
	cache_mat["OCCLUDE"] = mat_occlude;
}

void t_material::load (const std::string& path)
{
	std::ifstream f(path);

	if (!f)
		fatal("Material %s: cannot open file", path.c_str());

	name = path;

	std::string key;
	std::string value;

	struct bitmap_desc {
		std::string loc_name;
		GLuint texid;
	};
	std::vector<bitmap_desc> bitmaps;
	std::vector<GLuint> shaders;

	while (true) {
		f >> key >> value;
		f.ignore(-1, '\n'); // skip to next line
		if (!f)
			break;

		if (key == "FRAG") {
			GLuint s = get_frag_shader(value);
			shaders.push_back(s);
			fragment_shaders.push_back(s);
			continue;
		} else if (key == "VERT") {
			GLuint s = get_vert_shader(value);
			shaders.push_back(s);
			vertex_shaders.push_back(s);
			continue;
		}

		key = "map_" + key;
		bitmaps.push_back({ key, get_texture(value) });
	}

	// make sure any two materials with the same set of shaders
	// will have these vectors compare equal
	std::sort(fragment_shaders.begin(), fragment_shaders.end());
	std::sort(vertex_shaders.begin(), vertex_shaders.end());

	shaders.push_back(get_frag_shader("internal/material"));
	shaders.push_back(get_vert_shader("internal/material"));
	program = make_glsl_program(shaders);
	glUseProgram(program);

	glBindAttribLocation(program, ATTRIB_LOC_TANGENT, "tangent");

	int i = MAT_TEXTURE_SLOT_OFFSET;
	for (const auto& d: bitmaps) {
		int location = glGetUniformLocation(program,
				d.loc_name.c_str());

		if (location == -1) {
			warning("%s is not a valid uniform in material %s",
				d.loc_name.c_str(), path.c_str());
			continue;
		}

		glUniform1i(location, i++);
		bitmap_texture_ids.push_back(d.texid);
	}

	light_init_material();
}


/* Material application is idempotent, so we can avoid redundancy */
static const t_material* latest_material = nullptr;
static t_render_stage latest_render_stage;
void material_barrier ()
{
	latest_material = nullptr;
}

static bool should_skip_application (const t_material* m)
{
	t_render_stage s = latest_render_stage;
	if (s != render_ctx.stage || latest_material == nullptr)
		return false;

	if (m == latest_material)
		return true;

	if (s == RENDER_STAGE_LIGHTING_LSPACE
	|| s == RENDER_STAGE_WIREFRAME) {
		// these render stages only care about user vertex shaders
		if (latest_material->vertex_shaders == m->vertex_shaders)
			return true;
	}

	return false;
}


void t_material::apply () const
{
	if (should_skip_application(this))
		return;

	glUseProgram(program);

	for (int i = 0; i < bitmap_texture_ids.size(); i++) {
		bind_tex2d_to_slot(MAT_TEXTURE_SLOT_OFFSET + i,
				bitmap_texture_ids[i]);
	}

	render_ctx.submit_matrices();
	glUniform1i(UNIFORM_LOC_RENDER_STAGE, render_ctx.stage);
	light_apply_material();
}


GLenum get_surface_gl_format (SDL_Surface* s)
{
	auto compress = [] (uint32_t i) -> uint8_t {
		// 0x00FF00FF -> 0b0101 etc.
		i &= 0x08040201;
		i = i | (i >> 8) | (i >> 16) | (i >> 24);
		return i & 255;
	};

	uint16_t i = compress(s->format->Amask)
	          | (compress(s->format->Rmask) << 4)
	          | (compress(s->format->Gmask) << 8)
	          | (compress(s->format->Bmask) << 12);
	switch (i) {
	case 0b0001'0010'0100'0000:
		return GL_BGR;
	case 0b0100'0010'0001'0000:
		return GL_RGB;
	case 0b0001'0010'0100'1000:
		return GL_BGRA;
	case 0b0100'0010'0001'1000:
		return GL_RGBA;
	case 0b0000'0000'0001'0000:
		return GL_RED;
	case 0b0000'0010'0001'0000:
		return GL_RG;
	default:
		return -1;
	}
}

GLuint load_texture (std::string path)
{
	SDL_Surface* surf = IMG_Load(path.c_str());

	if (surf == nullptr)
		return 0;

	int format = get_surface_gl_format(surf);
	if (format == -1) {
		warning("Texture %s uses bogus format", path.c_str());
		SDL_FreeSurface(surf);
		return 0;
	}

	GLuint id;

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);

	glHint(GL_TEXTURE_COMPRESSION_HINT, GL_NICEST);

	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h,
			0, format, GL_UNSIGNED_BYTE, surf->pixels);

	SDL_FreeSurface(surf);

	return id;
}


GLuint make_glsl_program (const std::vector<GLuint>& shaders)
{
	GLuint r = glCreateProgram();

	for (GLuint s: shaders)
		glAttachShader(r, s);

	glLinkProgram(r);

	int link_success = 0;
	glGetProgramiv(r, GL_LINK_STATUS, &link_success);

	if (link_success)
		return r;

	int log_length = 0;
	glGetProgramiv(r, GL_INFO_LOG_LENGTH, &log_length);

	char log[log_length+1];
	log[log_length] = '\0';

	glGetProgramInfoLog(r, log_length, &log_length, log);
	fatal("OpenGL program %i failed to link. Log:\n%s\n", r, log);
}

GLuint compile_glsl (std::string path, GLenum type)
{
	std::ifstream f(path);

	if (!f)
		return 0;

	int filesize;
	f.seekg(0, f.end);
	filesize = f.tellg();
	f.seekg(0, f.beg);

	std::string source;

	for (std::string line; std::getline(f, line); ) {
		source += line;
		source += '\n';
	}

	GLuint id = glCreateShader(type);

	const char* ptr = source.c_str();
	glShaderSource(id, 1, &ptr, nullptr);
	glCompileShader(id);

	int success = 0;
	glGetShaderiv(id, GL_COMPILE_STATUS, &success);

	if (success)
		return id;

	int log_length = 0;
	glGetShaderiv(id, GL_INFO_LOG_LENGTH, &log_length);
	char log[log_length+1];
	log[log_length] = '\0';
	glGetShaderInfoLog(id, log_length, 0, log);

	warning("Failed to compile shader %s:\n%s\n",
			path.c_str(), log);
	return 0;
}

