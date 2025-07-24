#version 430 core
out vec4 FragColor;

in vec3 color;
in vec3 normal;
in vec3 world_frag_position;

uniform vec3 camera_position;
uniform vec3 light_direction;
uniform float bias;
uniform float view_distance;
uniform float fog_bias;
uniform vec3 fog_color;

void main()
{
    vec3 final_color = color;
    float diffuse = max(-dot(normal, normalize(light_direction)), bias);
    final_color *= diffuse;
    
    float distance = length(camera_position - world_frag_position);
    float exp = clamp((distance / view_distance) + fog_bias, 0.0f, 1.0f);
    float value = pow(4.0f, exp);
    float visibility = clamp(exp, 0.0f, 1.0f);

    FragColor = vec4(vec3(mix(final_color, fog_color, visibility)), 1.0f);
}