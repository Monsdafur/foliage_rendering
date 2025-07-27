
#version 430 core
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec2 a_uv;
layout (location = 2) in vec3 a_normal;
layout (location = 3) in vec3 a_color;

out vec3 color;

uniform mat4 projection;
uniform mat4 transform;

void main()
{
    vec4 world_position = transform * vec4(a_position.xyz, 1.0);
    gl_Position = projection *  world_position;
    color = a_color;
}