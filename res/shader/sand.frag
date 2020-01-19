#version 130

uniform sampler2D map_diffuse;
uniform sampler2D map_ao;

in vec2 tex_crd;
in vec3 world_pos;

vec4 surface_color ()
{
	vec4 clr = texture(map_diffuse, tex_crd);
	clr.rgb *= texture(map_ao, tex_crd).r;
	return clr;
}

vec3 surface_normal ()
{
	return vec3(0.0, 0.0, 1.0);
}
