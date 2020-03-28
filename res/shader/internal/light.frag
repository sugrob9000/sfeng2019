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

noperspective in vec2 texcrd;

const float DEPTH_BIAS = 3e-3;
const float MIN_FALLOFF = 2e-2;

vec3 light ()
{
	vec3 world_pos = texture(gbuffer_world_pos, texcrd).rgb;
	vec3 world_norm = texture(gbuffer_world_norm, texcrd).rgb;

	vec4 lspace_pos = light_view * vec4(world_pos, 1.0);
	vec3 lcoord = lspace_pos.xyz / lspace_pos.w;
	lcoord.z -= DEPTH_BIAS;

	if (lcoord.z > texture(depth_map, lcoord.st * 0.5 + 0.5).r)
		return vec3(0.0);

	float bright = max(0.0, 1.0 - length(lcoord.xy));
	if (bright == 0.0)
		return vec3(0.0);

	float lambert = max(0.0,
		dot(world_norm, normalize(light_pos - world_pos)));

	return bright * lambert * light_rgb;
}


void main ()
{
	gl_FragColor.rgb = texture(prev_shadowmap, texcrd).rgb + light();
}
