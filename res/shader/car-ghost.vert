#version 130

uniform uint stage;
#define LIGHTING_SSPACE 1u

/*
 * Receive lighting, but do not cast shadows
 * nor draw any actual pixels
 */
vec4 vertex_transform (vec4 modelspace_pos)
{
	if (stage == LIGHTING_SSPACE)
		return modelspace_pos;
	return vec4(0.0);
}
