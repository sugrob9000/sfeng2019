#version 130

/*
 * Simplest vertex shader that provides the vertex
 * position, normal, and texture coordinate just as
 * they were passed in from the model
 */

vec4 vertex_pos () { return gl_Vertex; }
vec3 vertex_norm () { return gl_Normal; }
vec2 vertex_texcoord () { return gl_MultiTexCoord0.st; }
