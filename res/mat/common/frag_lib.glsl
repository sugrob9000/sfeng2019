#version 130

/*
 * The user shader which links against the lib
 * should specify these
 */
lowp vec4 final_shade ();
vec3 surface_normal ();

const uint LIGHTING_LSPACE = 0u;
const uint LIGHTING_SSPACE = 1u;
const uint SHADE_FINAL = 2u;
uniform uint stage;

in vec3 world_normal;
in vec3 world_pos;

in vec3 lspace_pos;

void main ()
{
	switch (stage) {
	case LIGHTING_LSPACE:
		// do nothing except fill z-buffer
		break;

	case LIGHTING_SSPACE:
		// get the surface normal specified by
		// user shader and (TODO) calculate lighting
		vec3 norm = surface_normal();
		gl_FragColor = vec4(norm, 1.0);
		break;

	case SHADE_FINAL:
		// call the actual user shader
		gl_FragColor = final_shade();
		break;
	}
}

