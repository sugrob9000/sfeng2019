#version 330 core
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_explicit_attrib_location: require
#extension GL_ARB_draw_buffers: require

/*
 * The screenspace pass for the post-processing of light
 * Does nothing yet
 */

#include internal/light/_sspace_pass.inc
#include internal/_gbuffer.inc

void main ()
{
	OUT_DIFFUSE = texture(prev_diffuse_map, texcrd);
	OUT_SPECULAR = texture(prev_specular_map, texcrd);
}
