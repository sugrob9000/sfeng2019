#version 130
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_explicit_attrib_location: require

/* Debug - output a texture */

layout (location = 0) uniform vec3 xy_size;
noperspective out vec2 texcrd;

void main ()
{
	texcrd = gl_Vertex.xy;
	gl_Position.xy = texcrd * xy_size.z + xy_size.xy;
	gl_Position.zw = vec2(0.0, 1.0);
}
