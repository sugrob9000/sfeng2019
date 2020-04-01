#version 330 core
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_explicit_attrib_location: require
#extension GL_ARB_draw_buffers: require

layout (location = 0) uniform sampler2D prev_shadowmap;
layout (location = 1) uniform sampler2D depth_map;

layout (location = 200) uniform sampler2D gbuffer_world_pos;
layout (location = 201) uniform sampler2D gbuffer_world_norm;
layout (location = 202) uniform sampler2D gbuffer_screen_depth;

layout (location = 3) uniform vec3 eye_pos;
layout (location = 6) uniform vec3 light_pos;
layout (location = 9) uniform vec3 light_rgb;
layout (location = 12) uniform mat4 light_view;

const int NUM_CASCADES = 1;
layout (location = 100) uniform vec2 cascade_bound[2 * NUM_CASCADES];

noperspective in vec2 texcrd;

vec3 light ()
{
	vec3 world_pos = texture(gbuffer_world_pos, texcrd).rgb;
	vec3 world_norm = texture(gbuffer_world_norm, texcrd).rgb;
	float screen_depth = texture(gbuffer_screen_depth, texcrd).r;

	vec4 lspace_pos = light_view * vec4(world_pos, 1.0);
	if (lspace_pos.w < 0.0)
		return vec3(0.0);

	vec2 lcoord = lspace_pos.xy / lspace_pos.w;
	float bright = max(0.0, 1.0 - length(lcoord));
	if (bright == 0.0)
		return vec3(0.0);

	vec2 casc_low = cascade_bound[0];
	vec2 casc_high = cascade_bound[1];
	lcoord = (lcoord - casc_low) / (casc_high - casc_low);

	float lambert = max(0.0, dot(world_norm,
			normalize(light_pos - world_pos)));
	if (lambert == 0.0)
		return vec3(0.0);

	float depth = lspace_pos.w * (1.0 - 1e-2);
	if (texture(depth_map, lcoord).r < depth)
		return vec3(0.0);

	return bright * lambert * light_rgb;
}


void main ()
{
	gl_FragColor.rgb = texture(prev_shadowmap, texcrd).rgb + light();
}
