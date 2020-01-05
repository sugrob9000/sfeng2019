#version 130

uniform sampler2D map_diffuse;
uniform sampler2D map_normal;

in vec2 tex_crd;

vec3 get_illum ();

vec3 surface_normal ()
{
	vec3 f = texture(map_normal, tex_crd).rgb * 2.0 - 1.0;
	// ease the normal a bit
	f.z *= 1.4;
	return f;
}

vec4 final_shade ()
{
	vec4 clr = texture(map_diffuse, tex_crd);
	clr.rgb *= get_illum();
	return clr;
}

