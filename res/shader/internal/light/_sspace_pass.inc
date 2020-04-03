layout (location = 0) uniform sampler2D prev_diffuse_map;
layout (location = 1) uniform sampler2D prev_specular_map;

#define OUT_DIFFUSE gl_FragData[0].rgb
#define OUT_SPECULAR gl_FragData[1].rgb
