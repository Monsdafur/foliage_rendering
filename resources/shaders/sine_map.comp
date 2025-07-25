#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D image_output;

uniform float shift;
uniform vec2 wind_direction;

void main() {
    ivec2 texel_coord = ivec2(gl_GlobalInvocationID.xy);
    float theta = 0.25;
    float c = wind_direction.x;
    float s = wind_direction.y;
	float x = c * texel_coord.x + s * texel_coord.y;

    vec3 value = vec3((sin(x * 0.05f + shift) / 2.0 + 0.5));
	
    imageStore(image_output, texel_coord, vec4(value, 1.0));
}