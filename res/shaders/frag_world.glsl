#version 130

in vec2 tex_crd;
in vec3 normal;

void main ()
{
	vec3 clr = normal;
	clr += vec3(1.0);
	clr *= 0.5;

	gl_FragColor = vec4(clr, 1.0);
}

