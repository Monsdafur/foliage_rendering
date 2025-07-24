#version 430 core
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec3 a_color;

out vec3 color;
out vec3 normal;
out vec3 world_frag_position;

uniform mat4 projection;
uniform mat4 transform;

void main()
{
    vec4 world_position = transform * vec4(a_position.xyz, 1.0);
    world_frag_position = world_position.xyz;
    gl_Position = projection *  world_position;
    normal = normalize(mat3(transpose(inverse(transform))) * a_normal);
    if (dot(normal, vec3(0.0f, 1.0f, 0.0f)) < 0.2f) {
        color = a_color * clamp(world_position.y, 0.125f, 1.0f);
    }
    else {
        color = a_color;
    }
}