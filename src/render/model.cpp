#include "inc_general.h"
#include "model.h"
#include "render.h"
#include "material.h"
#include "resource.h"
#include "input/cmds.h"
#include <cassert>
#include <map>

void t_model_mem::gl_send_triangle (int tri_id) const
{
	for (int i = 0; i < 3; i++) {
		int idx = triangles[tri_id].index[i];

		const vec3& p = vertices[idx].v.pos;
		const vec3& n = vertices[idx].v.norm;
		const t_texcrd& t = vertices[idx].v.tex;
		const vec3& tg = vertices[idx].tangent;
		const vec3& b = vertices[idx].bitangent;

		glNormal3f(n.x, n.y, n.z);
		glVertexAttrib3f(attrib_loc_tangent, tg.x, tg.y, tg.z);
		glVertexAttrib3f(attrib_loc_bitangent, b.x, b.y, b.z);
		glTexCoord2f(t.u, 1.0 - t.v);
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
	for (const vert_internal& v: vertices)
		bbox.update(v.v.pos);

	// just in case, extend slightly
	bbox.start -= vec3(0.5);
	bbox.end += vec3(0.5);
}

void t_model_mem::load_obj (std::string path)
{
	std::ifstream f(path);
	if (!f)
		fatal("Could not open OBJ %s", path.c_str());

	std::vector<vec3> points;
	std::vector<vec3> normals;
	std::vector<t_texcrd> texcrds;

	std::map<t_vertex, int> vert_indices;

	t_material* current_material = mat_none;

	auto add_face = [&] (int* v, int* n, int* t)
	-> void {
		const vec3& v0 = points[v[0]];
		const vec3& v1 = points[v[1]];
		const vec3& v2 = points[v[2]];
		const t_texcrd& uv0 = texcrds[t[0]];
		const t_texcrd& uv1 = texcrds[t[1]];
		const t_texcrd& uv2 = texcrds[t[2]];
		vec3 d_pos1 = v1 - v0;
		vec3 d_pos2 = v2 - v0;
		t_texcrd d_uv1 = { uv1.u - uv0.u, uv1.v - uv0.v };
		t_texcrd d_uv2 = { uv2.u - uv0.u, uv2.v - uv0.v };
		vec3 tangent = d_pos1*d_uv1.v - d_pos2*d_uv1.v;
		vec3 bitangent = d_pos2*d_uv1.u - d_pos1*d_uv2.u;

		t_triangle tri = { { }, current_material };
		for (int i = 0; i < 3; i++) {
			t_vertex key = { points[v[i]],
			                 normals[n[i]],
					 texcrds[t[i]] };
			auto iter = vert_indices.find(key);
			if (iter == vert_indices.end()) {
				tri.index[i] = vertices.size();
				vertices.push_back(
					{ key, tangent, bitangent });
				vert_indices[key] = tri.index[i];
			} else {
				tri.index[i] = iter->second;
				vertices[tri.index[i]].tangent += tangent;
				vertices[tri.index[i]].bitangent += bitangent;
			}
		}
		triangles.push_back(tri);
	};

	auto pack =
		[] (char a, char b) constexpr -> uint16_t
		{ return ((a << 8) | b); };

	for (std::string line; std::getline(f, line); ) {

		int comment = line.find('#');
		if (comment != std::string::npos)
			line.erase(comment);

		if (line.size() < 2)
			continue;

		switch (pack(line[0], line[1])) {
		case pack('v', ' '): {
			// vertex
			float x, y, z;
			sscanf(line.c_str(), "%*s %f %f %f", &x, &y, &z);
			points.push_back({ x, y, z });
			break;
		}
		case pack('v', 'n'): {
			// vertex normal
			float x, y, z;
			sscanf(line.c_str(), "%*s %f %f %f", &x, &y, &z);
			normals.push_back({ x, y, z });
			break;
		}
		case pack('v', 't'): {
			// tex coord
			float u, v;
			sscanf(line.c_str(), "%*s %f %f", &u, &v);
			texcrds.push_back({ u, v });
			break;
		}
		case pack('f', ' '): {
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
		case pack('u', 's'): {
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

	for (vert_internal& v: vertices) {
		v.tangent.norm();
		v.bitangent.norm();
	}

	calc_bbox();
}

bool operator< (const t_texcrd& a, const t_texcrd& b)
{
	if (a.u == b.u)
		return a.v < b.v;
	return a.u < b.u;
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
	fatal("RVD not reimplemented");
	// model.dump_rvd(out);
}
