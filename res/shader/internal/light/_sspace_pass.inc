layout (location = 0) uniform sampler2D prev_diffuse_map;
layout (location = 1) uniform sampler2D prev_specular_map;
layout (location = 3) uniform vec3 eye_pos;

#define OUT_DIFFUSE gl_FragData[0].rgb
#define OUT_SPECULAR gl_FragData[1].rgb

#define IN_DIFFUSE texture(prev_diffuse_map, texcrd).rgb
#define IN_SPECULAR texture(prev_specular_map, texcrd).rgb
