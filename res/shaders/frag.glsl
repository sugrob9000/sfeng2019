#version 130

uniform sampler2D map_diffuse;

void main ()
{
	gl_FragColor = texture(map_diffuse, gl_TexCoord[0].st);
}
