#version 130

out vec2 tex_crd;
void main ()
{
	gl_Position = gl_ModelViewMatrix * gl_Vertex;
	tex_crd = gl_MultiTexCoord0.st;
}
