#version 130

/* Debug - output a texture */

uniform sampler2D tex;
noperspective in vec2 texcrd;

void main ()
{
	gl_FragColor.rgb = texture(tex, texcrd).rgb;
	gl_FragColor.a = 1.0;
}
