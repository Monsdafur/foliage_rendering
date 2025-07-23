#version 430 core
out vec4 FragColor;

in vec3 color;
in vec3 normal;

uniform vec3 light_direction;
uniform float bias;

void main()
{
    float d = max(-dot(normal, normalize(light_direction)), bias);
    FragColor = vec4(color * d, 1.0f);
}