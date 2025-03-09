#version 130
#extension GL_ARB_explicit_uniform_location: require
#extension GL_ARB_explicit_attrib_location: require

// Debug - output a texture to screen

layout (location = 3) uniform sampler2D tex;
noperspective in vec2 texcrd;

void main ()
{
  gl_FragColor.rgb = texture(tex, texcrd).rgb;
  gl_FragColor.a = 1.0;
}
