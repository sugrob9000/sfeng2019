#version 130

uniform sampler2D map_diffuse;
uniform sampler2D map_ao;

in vec2 tex_crd;
in vec3 world_pos;

vec3 get_illum ();

vec4 final_shade ()
{
	vec4 clr = texture(map_diffuse, tex_crd);
	clr.rgb *= texture(map_ao, tex_crd).r * get_illum();
	return clr;
}

vec3 surface_normal ()
{
	return vec3(0.0, 0.0, 1.0);
}
