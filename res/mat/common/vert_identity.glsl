#version 130

out vec2 tex_crd;
out vec3 world_normal;
out vec3 world_pos;

/*
 * Basic vertex shader that just applies the relevant matrices
 *   and sends useful info
 */
void main ()
{
	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;

	tex_crd = gl_MultiTexCoord0.st;
	world_normal = (gl_ModelViewMatrix * vec4(gl_Normal.xyz, 0.0)).xyz;
	world_pos = (gl_ModelViewMatrix * gl_Vertex).xyz;
}

