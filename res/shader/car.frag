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
	vec3 f = texture(map_normal, tex_crd).rgb;
	return f * 2.0 - 1.0;
}
