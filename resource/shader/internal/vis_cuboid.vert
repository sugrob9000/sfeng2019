#version 130
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_explicit_attrib_location: require

varying vec2 tex_crd;

layout (location = 100) uniform mat4 proj;
layout (location = 116) uniform mat4 view;

layout (location = 42) uniform vec3 cuboid[2];

void main ()
{
	vec3 center = (cuboid[0] + cuboid[1]) * 0.5;
	vec3 scale = (cuboid[1] - cuboid[0]) * 0.5;

	vec3 vert = gl_Vertex.xyz * scale + center;
	gl_Position = proj * view * vec4(vert, 1.0);
}
