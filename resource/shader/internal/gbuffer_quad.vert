#version 330 core
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_explicit_attrib_location: require

/*
 * G-buffer usage: just draw a quad over the whole screen.
 * Gets vertices in [-1, 1], sends also texcrd in [0, 1]
 */

noperspective out vec2 texcrd;

layout (location = 0) in vec2 coord;

void main ()
{
	gl_Position.xy = coord;
	gl_Position.zw = vec2(0.5, 1.0);
	texcrd = gl_Position.st * 0.5 + 0.5;
}
