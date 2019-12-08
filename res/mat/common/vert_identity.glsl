#version 130

out vec2 tex_crd;
out vec3 world_normal;
out vec3 world_pos;
out vec3 lspace_pos;

const uint LIGHTING_LSPACE = 0u;
const uint LIGHTING_SSPACE = 1u;
const uint SHADE_FINAL = 2u;
uniform uint stage;

uniform mat4 lspace_model;

/*
 * Basic vertex shader that just applies the relevant matrices
 *   and sends useful info
 */
void main ()
{
	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;

	tex_crd = gl_MultiTexCoord0.st;
	world_normal = (gl_ModelViewMatrix * vec4(gl_Normal, 0.0)).xyz;
	world_pos = (gl_ModelViewMatrix * gl_Vertex).xyz;

	if (stage == LIGHTING_SSPACE)
		lspace_pos = (lspace_model * gl_Vertex).xyz;
}
