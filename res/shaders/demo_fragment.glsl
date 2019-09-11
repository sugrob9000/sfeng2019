#version 130

in vec4 gl_FragCoord;

void main ()
{
	gl_FragColor.xyz = gl_FragCoord.xyz;
	gl_FragColor.w = 1.0;
}

