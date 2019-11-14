#version 130

out vec2 tex_crd;

/*
 * Basic vertex shader that just applies the relevant matrices
 */
void main ()
{
	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
	tex_crd = gl_MultiTexCoord0.st;
}
