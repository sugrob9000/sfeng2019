#version 330 core
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_explicit_attrib_location: require
#extension GL_ARB_draw_buffers: require


/* The user shader which links against the lib should implement these */
vec4 surface_color ();
vec3 surface_normal ();


/* ================================================== */

#define G_BUFFERS 0
#define LIGHTING_LSPACE 1
#define SHADE_FINAL 2
layout (location = 0) uniform int stage;

#define MRT_SLOT_WORLD_POS 0
#define MRT_SLOT_WORLD_NORM 1

in vec3 world_pos;
in vec4 screen_crd;
in mat3 TBN;

vec4 light_lspace ();

#define gl_FragColor gl_FragData[0]

void main ()
{
	switch (stage) {
	case G_BUFFERS:

		gl_FragData[MRT_SLOT_WORLD_POS].rgb = world_pos;
		gl_FragData[MRT_SLOT_WORLD_NORM].rgb =
			TBN * normalize(surface_normal());
		break;

	case LIGHTING_LSPACE:

		gl_FragColor = light_lspace();
		break;

	case SHADE_FINAL:

		// call the actual user shader
		gl_FragColor = surface_color();
		break;
	}
}

vec4 light_lspace ()
{
	return vec4(0.0);
}
