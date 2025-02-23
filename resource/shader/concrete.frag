#version 130

uniform sampler2D map_diffuse;
uniform sampler2D map_normal;

in vec2 tex_crd;

vec3 surface_normal ()
{
	vec3 f = texture(map_normal, tex_crd * 10.0).rgb;
	return f * 2.0 - 1.0;
}

vec4 surface_color ()
{
	return texture(map_diffuse, tex_crd);
}
