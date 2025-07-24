#version 430 core
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec3 a_color;

struct GrassBuffer {
    mat4 transform;
    mat4 sway;
};

layout(std430, binding = 0) buffer BufferData {
    GrassBuffer grass_buffer[];
};

out vec3 color;
out vec3 normal;
uniform mat4 projection;
uniform float offset;

void main()
{
    vec4 vector_offset = grass_buffer[gl_InstanceID].sway * normalize(vec4(0.4f, 0.0f, -1.2f, 0.0f)) * offset;
    vec4 world_position = grass_buffer[gl_InstanceID].transform * vec4(a_position, 1.0f) + vector_offset * a_position.y;
    gl_Position = projection * world_position;
    color = a_color * max(a_position.y, 0.5f);
}