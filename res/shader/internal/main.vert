#version 330 core
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_explicit_attrib_location: require


/* The user shader which links against the lib should implement these */
vec4 vertex_pos ();
vec3 vertex_norm ();
vec2 vertex_texcoord ();


/* ========================================== */

#define G_BUFFERS 0
#define LIGHTING_LSPACE 1
#define SHADE_FINAL 2
layout (location = 0) uniform int stage;

layout (location = 100) uniform mat4 proj;
layout (location = 116) uniform mat4 view;
layout (location = 132) uniform mat4 model;
attribute vec3 tangent;

out vec2 tex_crd;
out vec4 screen_crd;
out vec3 world_normal;
out vec3 world_pos;
out mat3 TBN;

void main ()
{
	vec4 pos = vertex_pos();
	vec4 normal = vec4(vertex_norm(), 0.0);
	tex_crd = vertex_texcoord();

	gl_Position = proj * view * model * pos;
	screen_crd = gl_Position;

	world_normal = (model * normal).xyz;
	world_pos = (model * pos).xyz;

	if (stage == G_BUFFERS) {
		vec3 w_tangent = (model * vec4(tangent, 0.0)).xyz;
		vec3 w_bitangent = cross(world_normal, w_tangent);
		TBN = mat3(w_tangent, w_bitangent, world_normal);
	}
}
