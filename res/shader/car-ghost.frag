#version 130

uniform sampler2D map_diffuse;
uniform sampler2D map_normal;

in vec2 tex_crd;

vec4 surface_color ()
{
	vec4 clr = texture(map_diffuse, tex_crd);
	return clr;
}

vec3 surface_normal ()
{
	vec3 f = texture(map_normal, tex_crd).rgb;
	return f * 2.0 - 1.0;
}
