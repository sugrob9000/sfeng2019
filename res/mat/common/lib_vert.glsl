#version 130

/* The user shader which links against the lib should implement this */
vec4 vertex_transform (vec4 input);

out vec2 tex_crd;
out vec3 world_normal;
out vec3 world_pos;
out vec3 lspace_pos;

attribute vec3 world_tangent;
out mat3 TBN;

const uint LIGHTING_LSPACE = 0u;
const uint LIGHTING_SSPACE = 1u;
const uint SHADE_FINAL = 2u;
uniform uint stage;

uniform mat4 lspace_model;

void main ()
{
	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix *
		vertex_transform(gl_Vertex);

	tex_crd = gl_MultiTexCoord0.st;
	world_normal = (gl_ModelViewMatrix * vec4(gl_Normal, 0.0)).xyz;
	world_pos = (gl_ModelViewMatrix * gl_Vertex).xyz;

	vec3 tangent = (gl_ModelViewMatrix * vec4(world_tangent, 0.0)).xyz;
	world_normal = normalize(world_normal);
	tangent = normalize(tangent);
	vec3 bitangent = normalize(cross(world_normal, tangent));

	TBN = mat3(tangent, bitangent, world_normal);

	if (stage == LIGHTING_SSPACE)
		lspace_pos = (lspace_model * gl_Vertex).xyz;
}
