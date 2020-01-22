#version 130

/*
 * Simple, conventional normal mapping
 */

uniform sampler2D map_normal;
in vec2 tex_crd;

vec3 surface_normal ()
{
	return texture(map_normal, tex_crd).rgb * 2.0 - 1.0;
}

