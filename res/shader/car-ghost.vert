#version 130

uniform uint stage;

/* Simple vertex shader that does nothing to its input */
vec4 vertex_transform (vec4 modelspace_pos)
{
	if (stage == 1u)
		return modelspace_pos;
	return vec4(0.0);
}
