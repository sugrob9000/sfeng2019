#version 130

uniform sampler2DRect tex;

varying vec2 tex_crd;

void main ()
{
	gl_FragColor = texture2DRect(tex, tex_crd);
}
