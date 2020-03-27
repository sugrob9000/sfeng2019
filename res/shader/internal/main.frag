#version 330 core
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_explicit_attrib_location: require
#extension GL_ARB_draw_buffers: require


/* The user shader which links against the lib should implement these */
vec4 surface_color ();
vec3 surface_normal ();


/* ================================================== */

#define RENDER_STAGE_G_BUFFERS 0
#define RENDER_STAGE_LIGHTING_LSPACE 1
#define RENDER_STAGE_SHADE_FINAL 2
#define RENDER_STAGE_WIREFRAME 3
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
	case RENDER_STAGE_G_BUFFERS:

		gl_FragData[MRT_SLOT_WORLD_POS].rgb = world_pos;
		gl_FragData[MRT_SLOT_WORLD_NORM].rgb =
			TBN * normalize(surface_normal());
		break;

	case RENDER_STAGE_LIGHTING_LSPACE:

		gl_FragColor = light_lspace();
		break;

	case RENDER_STAGE_SHADE_FINAL:

		// call the actual user shader
		gl_FragColor = surface_color();
		break;

	case RENDER_STAGE_WIREFRAME:

		gl_FragColor = vec4(1.0, 0.0, 0.0, 0.5);
		break;
	}
}

vec4 light_lspace ()
{
	return vec4(0.0);
}
