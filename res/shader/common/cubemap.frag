#version 130
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_explicit_attrib_location: require

/*
 * Get albedo from a texture
 */

uniform samplerCube cube_map;
in vec3 world_pos;
in vec3 world_center;

vec4 surface_color ()
{
	vec3 direction = normalize(world_pos - world_center);
	return texture(cube_map, direction);
	// return vec4(direction, 1.0);
}
vec3 surface_normal ()
{
	return vec3(0.0, 0.0, 1.0);
}