#version 130
varying vec2 tex_crd;

void main ()
{
	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
}
