#version 130

in vec2 tex_crd;
in vec3 world_normal;

vec3 get_illum ();

vec4 surface_color ()
{
	return vec4(world_normal * 0.5 + 0.5, 1.0);
}

vec3 surface_normal ()
{
	return vec3(0.0, 0.0, 1.0);
}

