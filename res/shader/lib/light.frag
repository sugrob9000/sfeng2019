#version 130
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_explicit_attrib_location: require


/* The user shader which links against the lib should implement these */
vec3 surface_normal ();
vec3 saturate_diffuse (float fac, vec3 color) { return color * fac; }
vec3 saturate_specular (float fac, vec3 color)
{
	return color * pow(fac, 5.0);
}


/* ================================================== */

in vec3 world_normal;
in vec3 world_pos;
in vec4 screen_crd;
in mat3 TBN;

layout (location = 1) uniform sampler2D prev_shadow_map;
layout (location = 10) uniform sampler2D depth_map;
layout (location = 11) uniform vec3 eye_pos;

layout (location = 12) uniform vec3 light_pos;
layout (location = 15) uniform vec3 light_rgb;
in vec4 lspace_pos;


float linstep (float low, float hi, float v);

const float EXP_FACTOR = 40.0;
const float NOBLEED_FACTOR = 0.4;
const float DEPTH_BIAS = 3e-4;

//#define LIGHT_BOX_BLUR

vec3 get_lighting ()
{
	vec2 s = screen_crd.xy / screen_crd.w * 0.5 + 0.5;
	return texture(prev_shadow_map, s).rgb;
}


/*
 * Calculate screenspace lighting (from player perspective)
 */
float chebyshev (vec2 moments, float depth);
vec3 light_sspace ()
{
	vec3 norm = TBN * normalize(surface_normal());

	vec3 lcoord = lspace_pos.xyz / lspace_pos.w;
	lcoord.z -= DEPTH_BIAS;
	float bright = max(1.0 - length(lcoord.xy), 0.0);

	vec4 moments;
#ifdef LIGHT_BOX_BLUR
	float samples = 4.0;
	float offset = 0.00195;
	float texel_size = 1.0 / 1024.0;
	for (float y = -offset; y <= offset; y += texel_size) {
		for (float x = -offset; x <= offset; x += texel_size) {
			vec2 crd = lcoord.st * 0.5 + 0.5 + vec2(x, y);
			moments += texture(depth_map, crd);
		}
	}
	moments /= samples*samples;
#else
	moments = texture(depth_map, lcoord.st * 0.5 + 0.5);
#endif

	float val_pos = exp(EXP_FACTOR * lcoord.z);
	float val_neg = -exp(-EXP_FACTOR * lcoord.z);
	float shadow_factor = min(chebyshev(moments.xy, val_pos),
	                          chebyshev(moments.zw, val_neg));

	vec3 diffuse, specular;
	float cos_diffuse, cos_specular;

	cos_diffuse = dot(norm, normalize(light_pos - world_pos));
	cos_diffuse = clamp(cos_diffuse, 0.0, 1.0);

	diffuse = saturate_diffuse(bright * cos_diffuse,
	                           shadow_factor * light_rgb);

	if (cos_diffuse > 0.0) {
		cos_specular = clamp(dot(
			reflect(normalize(world_pos - light_pos), norm),
			normalize(eye_pos - world_pos)), 0.0, 1.0);
	}
	specular = saturate_specular(bright * cos_specular,
	                             shadow_factor * light_rgb);

	return get_lighting() + diffuse + specular;
}

float chebyshev (vec2 moments, float depth)
{
	if (depth <= moments.x)
		return 1.0;

	float variance = moments.y - moments.x * moments.x;
	variance = max(variance, 0.0);

	float d = depth - moments.x;
	float p_max = variance / (variance + d * d);

	return linstep(NOBLEED_FACTOR, 1.0, p_max);
}



/*
 * Calculate lightspace EVSM map (from a light's perspective)
 */
vec4 light_lspace ()
{
	float depth = screen_crd.z / screen_crd.w;
	float pos = exp(EXP_FACTOR * depth);
	float neg = -exp(-EXP_FACTOR * depth);
	return vec4(pos, pos*pos, neg, neg*neg);
}

