#version 130

/* The user shade which links against the lib should implement this */
vec4 vertex_transform (vec4 v);

out vec2 tex_crd;
out vec4 screen_crd;
out vec3 world_normal;
out vec3 world_pos;

out vec4 lspace_pos;

attribute vec3 tangent;
out mat3 TBN;

#define LIGHTING_LSPACE 0u
#define LIGHTING_SSPACE 1u
#define SHADE_FINAL 2u
uniform uint stage;

uniform mat4 light_view;

void main ()
{
	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix *
		vertex_transform(gl_Vertex);

	tex_crd = gl_MultiTexCoord0.st;
	world_normal = (gl_ModelViewMatrix * vec4(gl_Normal, 0.0)).xyz;
	world_pos = (gl_ModelViewMatrix * gl_Vertex).xyz;

	screen_crd = gl_Position;

	if (stage == LIGHTING_SSPACE) {
		vec3 w_tangent =
			(gl_ModelViewMatrix * vec4(tangent, 0.0)).xyz;
		vec3 w_bitangent = cross(world_normal, w_tangent);
		TBN = mat3(w_tangent, w_bitangent, world_normal);

		lspace_pos = light_view * gl_ModelViewMatrix * gl_Vertex;
	}
}
