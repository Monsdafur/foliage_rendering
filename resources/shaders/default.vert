#version 430 core
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_color;

out vec3 color;
uniform mat4 projection;
uniform mat4 transform;

void main()
{
    vec4 position_local = projection * (transform * vec4(a_position.xyz, 1.0));
    gl_Position = position_local;
    color = a_color;
}