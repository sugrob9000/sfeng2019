#version 130

in vec3 world_normal;

vec4 surface_color ()
{
	return vec4(world_normal * 0.5 + 0.5, 1.0);
}
