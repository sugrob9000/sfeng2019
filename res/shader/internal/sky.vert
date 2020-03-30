#version 130
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_explicit_attrib_location: require

out float height_factor;

layout (location = 100) uniform mat4 proj;
layout (location = 116) uniform mat4 view;

void main ()
{
	mat4 view_notranslate = view;
	view_notranslate[3].xyz *= 0.0;
	gl_Position = proj * view_notranslate * gl_Vertex;
	gl_Position.z = gl_Position.z * 0.5 + 0.5;
	height_factor = gl_Vertex.z * 0.5;
}

