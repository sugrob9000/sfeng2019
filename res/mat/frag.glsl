#version 130

uniform sampler2D map_diffuse;
uniform sampler2D map_normal;

in vec2 tex_crd;
in vec3 world_normal;

vec4 final_shade ()
{
	return texture(map_diffuse, tex_crd);
}

vec3 surface_normal ()
{
	return normalize(texture(map_normal, tex_crd).rgb * 2 - 1);
}
