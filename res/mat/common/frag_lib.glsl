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

void main ()
{
	switch (stage) {
	case LIGHTING_LSPACE:
		// do nothing except fill z-buffer
		break;

	case LIGHTING_SSPACE:
		// get the surface normal specified by
		// user shader and (TODO) calculate lighting
		vec3 z = vec3(0.0, 0.0, 1.0);
		float c = dot(z, world_normal);
		float c1 = 1 - c;
		float ang = acos(c);
		vec3 axis = cross(z, world_normal);
		axis *= vec3(1.0, -1.0, 1.0);
		float s = sin(ang);
		float x2 = axis.x * axis.x;
		float y2 = axis.y * axis.y;
		float z2 = axis.z * axis.z;
		mat3 TBN = mat3(
				vec3(c + x2 * c1, axis.x * axis.y * c1 - axis.z * s, axis.x * axis.z * c1 + axis.y * s),
				vec3(axis.y * axis.x * c1 + axis.z * s, c + y2 * c1, axis.y * axis.z * c1 - axis.z * s),
				vec3(axis.z * axis.x * c1 - axis.y * s, axis.z * axis.y * c1 + axis.x * s, c + z2 * c1)
			);
		vec3 norm = TBN * surface_normal();
		gl_FragColor = vec4((norm + vec3(1))*0.5, 1.0);
		break;

	case SHADE_FINAL:
		// call the actual user shader
		gl_FragColor = final_shade();
		break;
	}
}

