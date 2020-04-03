#version 330 core
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_explicit_attrib_location: require
#extension GL_ARB_draw_buffers: require

/*
 * The screenspace pass for directional cone light
 */

#pragma include internal/light/_sspace_pass.inc
#pragma include internal/_gbuffer.inc

layout (location = 2) uniform sampler2D depth_map;

layout (location = 3) uniform vec3 eye_pos;
layout (location = 6) uniform vec3 light_pos;
layout (location = 9) uniform vec3 light_rgb;
layout (location = 12) uniform mat4 light_view;

layout (location = 100) uniform vec2 view_bound[2];

const float DEPTH_BIAS_MULTIPLIER = (1.0 - 1e-2);

void main ()
{
	vec3 world_pos = texture(gbuffer_world_pos, texcrd).rgb;
	vec3 world_norm = texture(gbuffer_world_norm, texcrd).rgb;
	float screen_depth = texture(gbuffer_screen_depth, texcrd).r;

	vec4 lspace = light_view * vec4(world_pos, 1.0);

	vec2 lcoord = lspace.xy / lspace.w;
	float bright = max(0.0, 1.0 - length(lcoord)) * step(0.0, lspace.w);

	vec2 casc_low = view_bound[0];
	vec2 casc_high =view_bound[1];
	lcoord = (lcoord - casc_low) / (casc_high - casc_low);

	bright *= max(0.0, dot(world_norm, normalize(light_pos - world_pos)));

	float depth = lspace.w * DEPTH_BIAS_MULTIPLIER;
	bright *= step(depth, texture(depth_map, lcoord).r);

	vec3 diffuse = bright * light_rgb;

	vec3 specular;
	if (bright > 0.0) {
		float exp = texture(gbuffer_specular, texcrd).r;
		float cos_spec = max(0.0, dot(reflect(
				normalize(world_pos - light_pos), world_norm),
				normalize(eye_pos - world_pos)));
		specular = bright * light_rgb * pow(cos_spec, exp);
	}

	OUT_SPECULAR = texture(prev_specular_map, texcrd).rgb + specular;
	OUT_DIFFUSE = texture(prev_diffuse_map, texcrd).rgb + diffuse;
}
