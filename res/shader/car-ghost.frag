#version 130

uniform sampler2D map_diffuse;
uniform sampler2D map_normal;

in vec2 tex_crd;
in vec3 world_normal;

vec3 get_illum ();

vec4 final_shade ()
{
	vec4 clr = texture(map_diffuse, tex_crd);
	clr.rgb *= get_illum();
	clr.a = 0.0;
	return clr;
}

vec3 surface_normal ()
{
	vec3 f = texture(map_normal, tex_crd).rgb;
	return f * 2.0 - 1.0;
}
