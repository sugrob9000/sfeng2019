#version 130

/* The user shader which links against the lib should implement these */
lowp vec4 final_shade ();
vec3 surface_normal ();

/* Branch on stage in main - shouldn't be that slow */
#define LIGHTING_LSPACE 0u
#define LIGHTING_SSPACE 1u
#define SHADE_FINAL 2u
uniform uint stage;

in vec3 world_normal;
in vec3 world_pos;
in mat3 TBN;

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
		break;
	default:
		gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
}

uniform mat4 lspace;
uniform sampler2D depth_map;
uniform sampler2D prev_shadow_map;
uniform vec3 light_pos;
uniform vec3 light_color;

in vec4 lspace_pos;
in vec4 sspace_pos;

vec3 sspace_light ()
{
	vec3 sn = normalize(surface_normal());
	vec3 norm = TBN * sn;

	vec3 lcoord = lspace_pos.xyz / lspace_pos.w;

	float bright = clamp(1.0 - length(lcoord.xy), 0.0, 1.0);
	lcoord *= 0.5;
	lcoord += 0.5;

	float cosine = dot(norm, normalize(light_pos - world_pos));

	float bias = max(3e-4, cosine * 3e-4);
	float d_current = lcoord.z - bias;
	float d_closest = texture(depth_map, lcoord.xy).x;

	float illum;
	if (d_current > d_closest)
		illum = 0.0;
	else
		illum = bright * clamp(cosine, 0.0, 1.0);

	vec2 scoord = (sspace_pos.xyz / sspace_pos.w).xy;
	scoord *= 0.5;
	scoord += 0.5;

	return light_color * illum + texture(prev_shadow_map, scoord.xy).rgb;
}
