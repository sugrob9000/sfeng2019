#version 130
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_explicit_attrib_location: require

/* Debug - output a texture */

layout (location = 3) uniform sampler2DArray tex;
layout (location = 4) uniform uint layer;
noperspective in vec2 texcrd;

void main ()
{
	gl_FragColor.rgb = texture(tex, vec3(texcrd, layer)).rgb;
	gl_FragColor.r *= 0.5;
	gl_FragColor.r += 0.5;

	gl_FragColor.a = 1.0;
}
