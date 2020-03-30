#version 330 core
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_explicit_attrib_location: require
#extension GL_ARB_draw_buffers: require

layout (location = 0) uniform sampler2D prev_shadowmap;
layout (location = 1) uniform sampler2D depth_map;

layout (location = 200) uniform sampler2D gbuffer_world_pos;
layout (location = 201) uniform sampler2D gbuffer_world_norm;

layout (location = 3) uniform vec3 eye_pos;
layout (location = 6) uniform vec3 light_pos;
layout (location = 9) uniform vec3 light_rgb;
layout (location = 12) uniform mat4 light_view;

const int NUM_CASCADES = 1;
layout (location = 100) uniform vec3 cascade_bound[2 * NUM_CASCADES];

noperspective in vec2 texcrd;

const float DEPTH_BIAS = 6e-4;
const float MIN_FALLOFF = 2e-2;

vec3 light ()
{
	vec3 world_pos = texture(gbuffer_world_pos, texcrd).rgb;
	vec3 world_norm = texture(gbuffer_world_norm, texcrd).rgb;

	vec4 lspace_pos = light_view * vec4(world_pos, 1.0);
	vec3 lcoord = lspace_pos.xyz / lspace_pos.w;
	float len_xy = length(lcoord.xy);

	vec3 casc_low = cascade_bound[0];
	vec3 casc_high = cascade_bound[1];

	lcoord = (lcoord - casc_low) / (casc_high - casc_low);
	lcoord.z -= DEPTH_BIAS;

	float bright = max(0.0, 1.0 - len_xy);
	bright *= max(0.0, dot(world_norm,
			normalize(light_pos - world_pos)));
	if (bright == 0.0)
		return vec3(0.0);

	if (texture(depth_map, lcoord.xy).r < lcoord.z)
		return vec3(0.0);

	return bright * light_rgb;
}


void main ()
{
	gl_FragColor.rgb = texture(prev_shadowmap, texcrd).rgb + light();
}
