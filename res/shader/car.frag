#version 130

uniform sampler2D map_diffuse;
uniform sampler2D map_normal;
uniform sampler2D map_diffuse_shift;

in vec2 tex_crd;
in vec3 world_normal;

vec3 get_illum ();

vec4 final_shade ()
{
	vec3 illum = get_illum();

	vec2 t = tex_crd + texture(map_diffuse_shift, tex_crd).rg * 1e-2;
	vec4 clr = texture(map_diffuse, t);
	clr.rgb *= illum;
	return clr;
}

vec3 surface_normal ()
{
	vec3 f = texture(map_normal, tex_crd).rgb;
	return f * 2.0 - 1.0;
}
