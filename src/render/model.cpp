#include "inc_general.h"
#include "model.h"
#include "render.h"

namespace render
{

t_model::t_model (std::string objpath)
{
	if (!load_obj(objpath))
		std::cout << "model loading failed" << std::flush;
}

void t_model::render () const
{
	glBegin(GL_TRIANGLES);
	for (const t_vertex v: verts) {
		glNormal3f(v.norm.x, v.norm.y, v.norm.z);
		glTexCoord2f(v.u, v.v);
		glVertex3f(v.pos.x, v.pos.y, v.pos.z);
	}
	glEnd();
}

bool t_model::load_obj (std::string path)
{
	std::ifstream f(path);
	if (!f)
		return false;

	struct texcoord {
		float u;
		float v;
	};

	std::vector<vec3> points;
	std::vector<vec3> normals;
	std::vector<texcoord> texcoords;

	auto encode2chars =
		[] (char a, char b) constexpr -> uint16_t
		{
			return ((a << 8) | b);
		};

	int faces = 0;

	int line_nr = 1;
	for (std::string line; std::getline(f, line); line_nr++) {

		int comment = line.find('#');
		if (comment != std::string::npos)
			line.erase(comment, std::string::npos);

		if (line.empty())
			continue;
		if (line.size() < 2)
			return false;

		uint16_t enc = encode2chars(line[0], line[1]);

		switch (enc) {
		case encode2chars('v', ' '): {
			// vertex
			float x, y, z;
			sscanf(line.c_str(), "%*s %f %f %f", &x, &y, &z);
			points.push_back({ x, y, z });
			break;
		}
		case encode2chars('v', 'n'): {
			// vertex normal
			float x, y, z;
			sscanf(line.c_str(), "%*s %f %f %f", &x, &y, &z);
			normals.push_back({ x, y, z });
			break;
		}
		case encode2chars('v', 't'): {
			// tex coord
			float u, v;
			sscanf(line.c_str(), "%*s %f %f", &u, &v);
			texcoords.push_back({ u, v });
			break;
		}
		case encode2chars('f', ' '): {
			// face
			int v[3];
			int n[3];
			int t[3];
			sscanf(line.c_str(), "%*s %i/%i/%i %i/%i/%i %i/%i/%i",
					&v[0], &t[0], &n[0],
					&v[1], &t[1], &n[1],
					&v[2], &t[2], &n[2]);

			for (int i = 0; i < 3; i++) {
				verts.push_back({
					points[v[i]-1],
					normals[v[i]-1],
					texcoords[t[i]-1].u,
					texcoords[t[i]-1].v });
			}

			break;
		}
		default: {
			// might be something in the format
			// we are unaware of
			continue;
		}
		}
	}

	return true;
}

bool t_model::load_rvd (std::string path)
{
	return false;
}

}
