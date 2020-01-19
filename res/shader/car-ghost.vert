#version 130

uniform uint stage;
#define LIGHTING_SSPACE 1u

/*
 * Receive lighting, but do not cast shadows
 * nor draw any actual pixels
 */

vec4 vertex_pos ()
{
	if (stage == LIGHTING_SSPACE)
		return gl_Vertex;
	return vec4(0.0);
}

vec3 vertex_norm () { return gl_Normal; }
vec2 vertex_texcoord () { return gl_MultiTexCoord0.st; }

