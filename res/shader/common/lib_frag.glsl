#version 130
#extension GL_ARB_explicit_uniform_location : require
#extension GL_ARB_explicit_attrib_location : require

/* The user shader which links against the lib should implement these */
lowp vec4 final_shade ();
vec3 surface_normal ();

/*
 * The locations must match the constants UNIFORM_LOC_*
 * specified in the source!
 */
#define LIGHTING_LSPACE 0u
#define LIGHTING_SSPACE 1u
#define SHADE_FINAL 2u
layout (location = 0) uniform uint stage;
layout (location = 1) uniform sampler2D prev_shadow_map;

layout (location = 2) uniform sampler2D depth_map;
layout (location = 3) uniform vec3 light_pos;
layout (location = 6) uniform vec3 light_rgb;
layout (location = 9) uniform mat4 light_view;

in vec3 world_normal;
in vec3 world_pos;
in mat3 TBN;
in vec4 screen_crd;

vec3 get_illum ();
vec3 sspace_light ();
void main ()
{
	switch (stage) {
	case LIGHTING_LSPACE:
		// do nothing except fill z-buffer
		break;
	case LIGHTING_SSPACE:
		gl_FragColor = vec4(sspace_light(), 1.0);
		break;
	case SHADE_FINAL:
		// call the actual user shader
		gl_FragColor = final_shade();
		gl_FragColor.rgb *= get_illum();
		break;
	default:
		gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
}

in vec4 lspace_pos;

vec3 sspace_light ()
{
	vec3 norm = TBN * normalize(surface_normal());
	vec3 lcoord = lspace_pos.xyz / lspace_pos.w;

	float bright = clamp(1.0 - length(lcoord.xy), 0.0, 1.0);
	lcoord *= 0.5;
	lcoord += 0.5;

	float cosine = dot(norm, normalize(light_pos - world_pos));

	float bias = max(3e-4, cosine * 3e-4);
	float d_current = lcoord.z - bias;
	float d_closest = texture(depth_map, lcoord.xy).x;

	float illum = (d_current > d_closest)
		? 0.0
		: bright * clamp(cosine, 0.0, 1.0);

	return light_rgb * illum + get_illum();
}

vec3 get_illum ()
{
	vec2 ss = (screen_crd.xyz / screen_crd.w).xy;
	ss *= 0.5;
	ss += 0.5;
	return texture(prev_shadow_map, ss).rgb;
}
