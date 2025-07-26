#version 430 core
out vec4 FragColor;

in vec2 uv;

uniform sampler2D diffuse_texture;

void main()
{
    FragColor = texture(diffuse_texture, uv);
}