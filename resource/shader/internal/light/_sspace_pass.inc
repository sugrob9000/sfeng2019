layout (location = 3) uniform vec3 eye_pos;

#define OUT_DIFFUSE gl_FragData[0].rgb
#define OUT_SPECULAR gl_FragData[1].rgb
