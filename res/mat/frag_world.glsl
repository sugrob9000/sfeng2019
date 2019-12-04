#version 130

in vec2 tex_crd;
in vec3 world_normal;

vec4 final_shade ()
{
	vec3 clr = world_normal;
	clr += vec3(1.0);
	clr *= 0.5;

	return vec4(clr, 1.0);
}

vec3 surface_normal ()
{
	return vec3(0.0, 0.0, 1.0);
}

