#version 130

uniform sampler2D map_diffuse;
uniform sampler2D map_normal;
uniform sampler2D map_ao;

in vec2 tex_crd;

vec4 final_shade ()
{
	float ao = texture(map_ao, tex_crd).r;
	vec4 clr = texture(map_diffuse, tex_crd * 10);
	clr.rgb *= ao;
	return clr;
}

vec3 surface_normal ()
{
	vec3 f = texture(map_normal, tex_crd * 10).rgb;
	return f * 2.0 - 1.0;
}

