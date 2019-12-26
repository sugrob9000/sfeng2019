#version 130

/* Simple vertex shader that does nothing to its input */
vec4 vertex_transform (vec4 modelspace_pos)
{
	return modelspace_pos;
}
