#version 130

uniform sampler2D map_diffuse;
uniform sampler2D map_ao;

in vec2 tex_crd;
in vec3 world_pos;

void main ()
{
	lowp vec3 clr = texture(map_diffuse, tex_crd).rgb;
	clr *= texture(map_ao, tex_crd).r;

	gl_FragColor = vec4(clr, 1.0);
}

