#version 130

uniform sampler2D map_diffuse;

in vec2 tex_crd;

vec4 final_shade ()
{
	return texture(map_diffuse, tex_crd);
}

vec3 surface_normal ()
{
	return vec3(0.0, 0.0, 1.0);
}
