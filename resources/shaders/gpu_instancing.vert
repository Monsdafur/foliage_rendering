#version 430 core
layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;
layout (location = 2) in vec3 a_color;
layout(std430, binding = 0) buffer BufferData {
    mat4 transform[];
};

out vec3 color;
out vec3 normal;
uniform mat4 projection;

void main()
{
    gl_Position = projection * transform[gl_InstanceID] * vec4(a_position.x, a_position.y, 0.0, 1.0);
    color = a_color;
}