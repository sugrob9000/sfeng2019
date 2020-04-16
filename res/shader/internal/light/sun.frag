#version 330 core
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_explicit_attrib_location: require
#extension GL_ARB_draw_buffers: require

/*
 * The screenspace pass for directional sunlight
 * (parallel light rays everywhere)
 */

#include internal/light/_sspace_pass.inc
#include internal/_gbuffer.inc

const int sun_num_cascades = 3;

layout (location = 2) uniform sampler2DArray depth_map;

layout (location = 6) uniform vec3 light_rgb;
layout (location = 9) uniform mat4 light_view[sun_num_cascades];
layout (location = 25) uniform vec3 light_direction;
layout (location = 30) uniform float depths[sun_num_cascades + 1];

const float DEPTH_BIAS = 3e-3;

int get_cascade ();
void main ()
{
	vec3 world_pos = texture(gbuffer_world_pos, texcrd).rgb;
	vec3 world_norm = texture(gbuffer_world_norm, texcrd).rgb;

	int casc = get_cascade();

	vec4 lspace = light_view[casc] * vec4(world_pos, 1.0);
	vec3 lcoord = lspace.xyz;
	lcoord.z -= DEPTH_BIAS;
	lcoord.xy = lcoord.xy * 0.5 + 0.5;

	vec3 diffuse = vec3(0.0);
	vec3 specular = vec3(0.0);

	if (lcoord.z <= texture(depth_map, vec3(lcoord.xy, casc)).r) {
		float bright = max(0.0, dot(world_norm, light_direction));
		diffuse = light_rgb * bright;

		float exp = texture(gbuffer_specular, texcrd).r;
		float cos_spec = max(0.0, dot(
				reflect(-light_direction, world_norm),
				normalize(eye_pos - world_pos)));
		specular = light_rgb * bright * pow(cos_spec, exp);
	}

	OUT_DIFFUSE = IN_DIFFUSE + diffuse;
	OUT_SPECULAR = IN_SPECULAR + specular;
}

int get_cascade ()
{
	float d = texture(gbuffer_screen_depth, texcrd).r;
	int i = 0;
	for (; i < sun_num_cascades; i++) {
		if (depths[i] > d)
			break;
	}
	return i-1;
}
