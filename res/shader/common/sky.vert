#version 130

out float height_factor;

void main ()
{
	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
	height_factor = gl_Vertex.z * 0.5;
}

