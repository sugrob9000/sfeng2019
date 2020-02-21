#version 130
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_explicit_attrib_location: require


/* The user shader which links against the lib should implement these */
vec4 vertex_pos ();
vec3 vertex_norm ();
vec2 vertex_texcoord ();


/* ========================================== */

out vec2 tex_crd;
out vec4 screen_crd;
out vec3 world_normal;
out vec3 world_pos;
out vec3 world_center;

attribute vec3 tangent;
out mat3 TBN;

#define LIGHTING_LSPACE 0u
#define LIGHTING_SSPACE 1u
#define SHADE_FINAL 2u
#define REF_RENDER 3u
uniform uint stage;

layout (location = 18) uniform mat4 light_view;
out vec4 lspace_pos;

#define MODEL gl_ModelViewMatrix
#define VIEWPROJ gl_ProjectionMatrix

void main ()
{
	vec4 pos = vertex_pos();
	vec4 normal = vec4(vertex_norm(), 0.0);
	tex_crd = vertex_texcoord();

	gl_Position = VIEWPROJ * MODEL * pos;
	screen_crd = gl_Position;

	world_normal = (MODEL * normal).xyz;
	world_pos = (MODEL * pos).xyz;
	world_center = (MODEL * vec4(vec3(0.0), 1.0)).xyz;

	if (stage == LIGHTING_SSPACE) {
		vec3 w_tangent = (MODEL * vec4(tangent, 0.0)).xyz;
		vec3 w_bitangent = cross(world_normal, w_tangent);
		TBN = mat3(w_tangent, w_bitangent, world_normal);
		lspace_pos = light_view * MODEL * pos;
	}
}
