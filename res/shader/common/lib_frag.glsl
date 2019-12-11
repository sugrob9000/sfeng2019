#version 130

/* The user shader which links against the lib should implement these */
lowp vec4 final_shade ();
vec3 surface_normal ();


const uint LIGHTING_LSPACE = 0u;
const uint LIGHTING_SSPACE = 1u;
const uint SHADE_FINAL = 2u;

uniform uint stage;

in vec3 world_normal;
in vec3 world_pos;

in vec3 lspace_pos;
in mat3 TBN;

void main ()
{
	switch (stage) {
	case LIGHTING_LSPACE:
		// do nothing except fill z-buffer
		break;

	case LIGHTING_SSPACE:
		vec3 sn = normalize(surface_normal());
		vec3 final_norm = TBN * sn;
		gl_FragColor = vec4((final_norm + 1.0) / 2, 1.0);
		break;

	case SHADE_FINAL:
		// call the actual user shader
		gl_FragColor = final_shade();
		break;

	default:
		gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
	}
}

