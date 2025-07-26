#version 430 core
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_uv;
layout (location = 2) in vec3 a_normal;
layout (location = 3) in vec3 a_color;

struct GrassBuffer {
    mat4 transform;
    mat4 sway;
};

layout(std430, binding = 0) buffer BufferData {
    GrassBuffer grass_buffer[];
};

out vec3 color;
out vec3 normal;
out vec3 world_frag_position;
out vec2 uv;

uniform mat4 projection;
uniform float offset;
uniform vec2 wind_direction;

void main()
{
    float offset = grass_buffer[gl_InstanceID].sway[0][1] * 2.0 - 1.0;
    vec4 vector_offset = vec4(wind_direction.x, 0.0f, wind_direction.y, 0.0f) * offset;
    vec4 world_position = (grass_buffer[gl_InstanceID].transform * vec4(a_position, 1.0f)) + vector_offset * a_position.y;
    world_frag_position = world_position.xyz;
    gl_Position = projection *  world_position;
    normal = normalize(mat3(transpose(inverse(grass_buffer[gl_InstanceID].transform))) * a_normal);
    color = a_color * clamp(world_position.y - grass_buffer[gl_InstanceID].sway[3][2], 0.125f, 1.0f);
    // color = vec3(grass_buffer[gl_InstanceID].sway[1][1]);
    // color = vec3(grass_buffer[gl_InstanceID].sway[3][0], grass_buffer[gl_InstanceID].sway[3][1], 0.0);
    uv = a_uv;
}