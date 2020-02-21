#version 130
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_explicit_attrib_location: require


/* The user shader which links against the lib should implement these */
vec4 surface_color ();


/* ================================================== */

#define MODEL gl_ModelViewMatrix
#define VIEWPROJ gl_ProjectionMatrix

#define LIGHTING_LSPACE 0u
#define LIGHTING_SSPACE 1u
#define SHADE_FINAL 2u
#define REF_RENDER 3u

layout (location = 0) uniform uint stage;

in vec3 world_pos;
in vec4 screen_crd;

vec3 get_lighting ();
vec3 light_sspace ();
vec4 light_lspace ();

float linstep (float low, float hi, float v)
{
	return clamp((v - low) / (hi - low), 0.0, 1.0);
}

const float FOG_HEIGHT_MAX = -500.0;
const float FOG_HEIGHT_MIN = 75.0;
const float FOG_DEPTH_MIN = 0.999;
const float FOG_DEPTH_MAX = 1.0;
const vec3 FOG_COLOR = vec3(0.28, 0.28, 0.42);

void main ()
{
	switch (stage) {
	case LIGHTING_LSPACE:

		gl_FragColor = light_lspace();
		break;

	case LIGHTING_SSPACE:

		gl_FragColor.rgb = light_sspace();
		break;

	case SHADE_FINAL:

		// call the actual user shader
		gl_FragColor = surface_color();
		gl_FragColor.rgb *= get_lighting();

		// dumb fog
		float fog = linstep(FOG_DEPTH_MIN, FOG_DEPTH_MAX,
				screen_crd.z / screen_crd.w);
		fog += linstep(FOG_HEIGHT_MIN, FOG_HEIGHT_MAX, world_pos.z);
		gl_FragColor.rgb = mix(gl_FragColor.rgb, FOG_COLOR,
				clamp(fog, 0.0, 1.0));

		break;

	case REF_RENDER:

		gl_FragColor = surface_color();

		break;
	}
}
