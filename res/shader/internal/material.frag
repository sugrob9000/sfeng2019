#version 330 core
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_explicit_attrib_location: require
#extension GL_ARB_draw_buffers: require


/*
 * ==================================================
 * These functions should be implemented by the
 * user material shader that links against this
 */

/* Called during g-buffer stage */
vec3 surface_normal ();
float specular_exponent () { return 90.0; }

/* Called during final shading stage */
vec4 surface_color ();

/* ================================================== */

#define RENDER_STAGE_G_BUFFERS 0
#define RENDER_STAGE_LIGHTING_LSPACE 1
#define RENDER_STAGE_SHADE_FINAL 2
#define RENDER_STAGE_WIREFRAME 3

layout (location = 0) uniform int stage;

layout (location = 1) uniform sampler2D lightmap_diffuse;
layout (location = 2) uniform sampler2D lightmap_specular;

in vec3 world_pos;
in vec3 world_normal;
in vec4 screen_crd;
in mat3 TBN;

#define gl_FragColor gl_FragData[0]

void main ()
{
	switch (stage) {
	case RENDER_STAGE_G_BUFFERS:

		#define GBUF_WORLD_POS gl_FragData[0].rgb
		#define GBUF_WORLD_NORM gl_FragData[1].rgb
		#define GBUF_SPECULAR_DATA gl_FragData[2].r

		GBUF_WORLD_POS = world_pos;
		GBUF_WORLD_NORM = TBN * normalize(surface_normal());
		GBUF_SPECULAR_DATA = specular_exponent();

		break;

	case RENDER_STAGE_LIGHTING_LSPACE:

		gl_FragColor.r = screen_crd.w;
		break;

	case RENDER_STAGE_SHADE_FINAL:

		vec2 texcrd = screen_crd.xy / screen_crd.w * 0.5 + 0.5;

		// call the actual user shader
		gl_FragColor = surface_color();

		vec3 light = texture(lightmap_diffuse, texcrd).rgb;
		light += texture(lightmap_specular, texcrd).rgb;

		gl_FragColor.rgb *= light;
		break;

	case RENDER_STAGE_WIREFRAME:

		gl_FragColor = vec4(1.0, 0.0, 0.0, 0.5);
		break;
	}
}
