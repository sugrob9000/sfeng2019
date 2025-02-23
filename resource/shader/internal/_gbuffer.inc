layout (location = 200) uniform sampler2D gbuffer_world_pos;
layout (location = 201) uniform sampler2D gbuffer_world_norm;
layout (location = 202) uniform sampler2D gbuffer_screen_depth;
layout (location = 203) uniform sampler2D gbuffer_specular;

noperspective in vec2 texcrd;
