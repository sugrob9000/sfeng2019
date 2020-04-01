#include "render/model.h"
#include "render/render.h"
#include "render/material.h"
#include "render/resource.h"
#include "input/cmds.h"
#include <cassert>
#include <map>

void t_model_mem::gl_send_triangle (int tri_id) const
{
	for (int i = 0; i < 3; i++) {
		int idx = triangles[tri_id].index[i];

		const vec3& p = vertices[idx].v.pos;
		const vec3& n = vertices[idx].v.norm;
		const vec2& t = vertices[idx].v.tex;
		const vec3& tg = vertices[idx].tangent;

		glNormal3f(n.x, n.y, n.z);
		glVertexAttrib3f(ATTRIB_LOC_TANGENT, tg.x, tg.y, tg.z);
		glTexCoord2f(t.x, 1.0 - t.y);
		glVertex3f(p.x, p.y, p.z);
	}
}

void t_model::render () const
{
	glCallList(display_list_id);
}

void t_model::load (const t_model_mem& src)
{
	display_list_id = glGenLists(1);
	glNewList(display_list_id, GL_COMPILE);
	glBegin(GL_TRIANGLES);
	int n = src.triangles.size();
	for (int i = 0; i < n; i++)
		src.gl_send_triangle(i);
	glEnd();
	glEndList();

	bbox = src.bbox;
}



void t_model_mem::calc_bbox ()
{
	if (vertices.empty())
		return;

	bbox = { vec3(INFINITY), vec3(-INFINITY) };
	for (const vertex& v: vertices)
		bbox.expand(v.v.pos);

	// just in case, extend slightly
	bbox.start -= vec3(0.5);
	bbox.end += vec3(0.5);
}

void t_model_mem::load_obj (const std::string& path)
{
	std::ifstream f(path);
	if (!f)
		fatal("Could not open OBJ %s", path.c_str());

	std::vector<vec3> points;
	std::vector<vec3> normals;
	std::vector<vec2> texcrds;

	std::map<t_vertex, int> vert_indices;

	t_material* current_material = mat_none;

	auto add_face = [&] (int* v, int* n, int* t)
	-> void {
		vec3 d_pos1 = points[v[1]] - points[v[0]];
		vec3 d_pos2 = points[v[2]] - points[v[0]];
		vec2 d_uv1 = texcrds[t[1]] - texcrds[t[0]];
		vec2 d_uv2 = texcrds[t[2]] - texcrds[t[0]];

		vec3 tangent = d_uv2.y * d_pos1 - d_uv1.y * d_pos2;

		triangle tri = { { }, current_material };
		for (int i = 0; i < 3; i++) {
			t_vertex key = { points[v[i]],
			                 normals[n[i]],
					 texcrds[t[i]] };
			auto iter = vert_indices.find(key);
			if (iter == vert_indices.end()) {
				tri.index[i] = vertices.size();
				vertices.push_back({ key, tangent });
				vert_indices[key] = tri.index[i];
			} else {
				tri.index[i] = iter->second;
				vertices[tri.index[i]].tangent += tangent;
			}
		}
		triangles.push_back(tri);
	};

	auto pack = [] (const char* s)
	constexpr -> uint16_t {
		return (s[0] << 8) | s[1];
	};

	for (std::string line; std::getline(f, line); ) {

		int comment = line.find('#');
		if (comment != std::string::npos)
			line.erase(comment);

		if (line.size() < 2)
			continue;

		switch (pack(line.c_str())) {
		case pack("v "): {
			// vertex
			float x, y, z;
			sscanf(line.c_str(), "%*s %f %f %f", &x, &y, &z);
			points.push_back({ x, y, z });
			break;
		}
		case pack("vn"): {
			// vertex normal
			float x, y, z;
			sscanf(line.c_str(), "%*s %f %f %f", &x, &y, &z);
			normals.push_back({ x, y, z });
			break;
		}
		case pack("vt"): {
			// tex coord
			float u, v;
			sscanf(line.c_str(), "%*s %f %f", &u, &v);
			texcrds.push_back({ u, v });
			break;
		}
		case pack("f "): {
			// face
			int v[3];
			int n[3];
			int t[3];
			sscanf(line.c_str(), "%*s %i/%i/%i %i/%i/%i %i/%i/%i",
					&v[0], &t[0], &n[0],
					&v[1], &t[1], &n[1],
					&v[2], &t[2], &n[2]);
			for (int i = 0; i < 3; i++) {
				v[i]--;
				n[i]--;
				t[i]--;
			}
			add_face(v, n, t);
			break;
		}
		case pack("us"): {
			// usemtl - update current material
			char buf[line.length()];
			sscanf(line.c_str(), "%*s %s", buf);
			current_material = get_material(buf);
		}
		default: {
			// something in the format we are unaware of
			continue;
		}
		}
	}

	for (vertex& v: vertices)
		v.tangent = glm::normalize(v.tangent);

	calc_bbox();
}


void t_model_mem::load_rvd (const std::string& path)
{
	std::ifstream f(path, std::ios::binary);
	if (!f)
		fatal("Model RVD %s: could not open file", path.c_str());

	uint32_t num_vertices = 0;
	f.read((char*) &num_vertices, sizeof(num_vertices));

	vertices.resize(num_vertices);
	f.read((char*) vertices.data(), sizeof(vertex) * num_vertices);

	uint32_t num_buckets = 0;
	f.read((char*) &num_buckets, sizeof(num_buckets));

	for (int i = 0; i < num_buckets; i++) {
		uint32_t name_len = 0;
		f.read((char*) &name_len, sizeof(name_len));

		std::string mat_name(name_len, '\0');
		f.read(mat_name.data(), name_len);

		t_material* mat = get_material(mat_name);

		uint32_t num_verts = 0;
		f.read((char*) &num_verts, sizeof(num_verts));
		assert(num_verts % 3 == 0);

		triangles.reserve(triangles.size() + num_verts);
		for (int j = 0; j < num_verts; j += 3) {
			triangles.push_back({ { }, mat });
			triangle& tri = triangles.back();
			f.read((char*) tri.index, 3 * sizeof(tri.index[0]));
		}
	}

	calc_bbox();
}

void t_model_mem::dump_rvd (const std::string& path) const
{
	std::ofstream f(path, std::ios::binary);
	if (!f) {
		fatal("Model RVD dump: could not open file %s for writing",
				path.c_str());
	}

	uint32_t num_vertices = vertices.size();
	f.write((const char*) &num_vertices, sizeof(num_vertices));
	f.write((const char*) vertices.data(), sizeof(vertex) * num_vertices);

	std::map<t_material*, std::vector<int>> m;
	for (const triangle& t: triangles) {
		std::vector<int>& v = m[t.material];
		for (int i = 0; i < 3; i++)
			v.push_back(t.index[i]);
	}

	uint32_t num_buckets = m.size();
	f.write((const char*) &num_buckets, sizeof(num_buckets));

	for (const auto& [mat, indices]: m) {
		uint32_t name_len = mat->name.length();
		f.write((const char*) &name_len, sizeof(name_len));
		f.write(mat->name.data(), name_len);

		uint32_t num_verts = indices.size();
		f.write((const char*) &num_verts, sizeof(num_verts));
		f.write((const char*) indices.data(),
				sizeof(indices[0]) * num_verts);
	}
}


COMMAND_ROUTINE (obj2rvd)
{
	if (ev != PRESS)
		return;
	if (args.empty())
		return;

	const std::string& in = args[0];
	std::string out;

	if (args.size() > 1) {
		out = args[1];
	} else {
		// add .rvd at the end or instead of .obj
		out = in;
		int size = out.size();
		if (size > 4 && out.compare(size-4, 4, ".obj") == 0)
			out.erase(size-4, std::string::npos);
		out += ".rvd";
	}

	t_model_mem model;
	model.load_obj(in);
	model.dump_rvd(out);
}


bool operator< (const t_vertex& a, const t_vertex& b)
{
	if (a.pos == b.pos) {
		if (a.norm == b.norm)
			return a.tex < b.tex;
		return a.norm < b.norm;
	}
	return a.pos < b.pos;
}
