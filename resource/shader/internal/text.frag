#version 130

uniform sampler2DRect glyphs;

varying vec2 tex_crd;

void main ()
{
	gl_FragColor = texture2DRect(glyphs, tex_crd);
}
