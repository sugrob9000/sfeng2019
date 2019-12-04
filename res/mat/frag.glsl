#version 130

uniform sampler2D map_diffuse;

in vec2 tex_crd;
in vec3 world_normal;

vec4 final_shade ()
{
	// return texture(map_diffuse, tex_crd);
	vec3 clr = world_normal;
	clr += vec3(1.0);
	clr *= 0.5;
	return vec4(clr, 1.0);
}

vec3 surface_normal ()
{
	return vec3(0.0, 0.0, 1.0);
}
