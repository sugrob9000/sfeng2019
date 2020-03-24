#version 130

in float height_factor;

void main ()
{
	vec3 clr_top = vec3(0.5, 0.7, 0.95);
	vec3 clr_bottom = vec3(0.28, 0.28, 0.42);

	float factor = height_factor + 0.5;
	gl_FragColor = vec4(clr_bottom + factor * (clr_top - clr_bottom), 1.0);
}

