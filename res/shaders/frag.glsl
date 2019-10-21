#version 130

uniform sampler2D map_diffuse;

in vec2 tex_crd;

void main ()
{
	gl_FragColor = texture(map_diffuse, tex_crd);
}
