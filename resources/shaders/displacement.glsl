#version 430 core
layout(local_size_x = 1, local_size_y = 1) in;

struct GrassBuffer {
    mat4 transform;
    mat4 sway;
};

layout(std430, binding = 0) buffer BufferData {
    GrassBuffer grass_buffer[];
};

uniform int width;
uniform int height;
uniform float field_size;
uniform vec3 start_position;
uniform sampler2D noise_map;

void main() {
    uvec2 id = gl_GlobalInvocationID.xy;
    if (id.x < width && id.y < height) {
        uint index = id.y * uint(width) + id.x;
        vec2 uv = vec2(float(id.x) / float(width), float(id.y) / float(height));
        grass_buffer[index].sway[0][1] = grass_buffer[index].sway[0][0] * (texture(noise_map, uv).r - 0.5f);
        grass_buffer[index].sway[1][1] = texture(noise_map, uv).r;
    }
}