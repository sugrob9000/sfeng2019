#version 130

/*
 * The user shader which links against the lib
 * should specify these
 */
lowp vec4 final_shade ();
vec3 surface_normal ();

/*
 * True when we are generating the shadow buffer for the frame
 */
uniform bool light_pass;

in vec3 world_normal;
in vec3 world_pos;

void main ()
{
	if (light_pass) {
		vec3 norm = surface_normal();
		gl_FragColor = vec4(norm, 1.0);
	} else {
		gl_FragColor = final_shade();
	}
}

