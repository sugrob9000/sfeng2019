#version 130

uniform sampler2D map_diffuse;
uniform sampler2D map_normal;
uniform sampler2D map_ao;

in vec2 tex_crd;

vec3 surface_normal ()
{
	vec3 f = texture(map_normal, tex_crd * 10).rgb;
	return f * 2.0 - 1.0;
}

vec4 final_shade ()
{
	float ao = texture(map_ao, tex_crd).r;
	vec2 t = tex_crd * 10;
	t += surface_normal().xy;
	vec4 clr = texture(map_diffuse, t);
	clr.rgb *= ao;
	return clr;
}

