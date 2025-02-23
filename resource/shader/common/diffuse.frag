#version 130

/*
 * Get albedo from a texture
 */

uniform sampler2D map_diffuse;
in vec2 tex_crd;

vec4 surface_color ()
{
	return texture(map_diffuse, tex_crd);
}
