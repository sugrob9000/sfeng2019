#version 130

/*
 * G-buffer usage: just draw a quad over the whole screen.
 * Gets vertices in [-1, 1], sends also texcrd in [0, 1]
 */

noperspective out vec2 texcrd;

void main ()
{
	gl_Position = gl_Vertex;
	texcrd = gl_Position.st * 0.5 + 0.5;
}
