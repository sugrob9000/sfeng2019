#version 130
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_explicit_attrib_location: require

varying vec2 tex_crd;

layout (location = 100) uniform mat4 proj;
layout (location = 116) uniform mat4 view;

void main ()
{
	gl_Position = proj * view * gl_Vertex;
}
