#version 330 core
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_explicit_attrib_location: require
#extension GL_ARB_draw_buffers: require

/*
 * The screenspace pass for the post-processing of light
 * Does nothing yet
 */

layout (location = 1) uniform sampler2D prev_diffuse_map;
layout (location = 2) uniform sampler2D prev_specular_map;

layout (location = 202) uniform sampler2D gbuffer_screen_depth;

noperspective in vec2 texcrd;

#define OUT_DIFFUSE gl_FragData[0].rgb
#define OUT_SPECULAR gl_FragData[1].rgb

void main ()
{
	OUT_DIFFUSE = texture(prev_diffuse_map, texcrd);
	OUT_SPECULAR = texture(prev_specular_map, texcrd);
}
