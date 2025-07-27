#version 430 core
out vec4 FragColor;

in vec3 color;
in vec3 normal;
in vec3 world_frag_position;
in vec2 uv;

uniform int has_texture;
uniform sampler2D diffuse_texture;

uniform vec3 camera_position;
uniform vec3 light_direction;
uniform float bias;
uniform float view_distance;
uniform float fog_bias;
uniform vec3 fog_color;
uniform int disable_fog;

void main()
{
    vec3 texture_color = color;
    if (has_texture > 0) {
        texture_color *= vec3(texture(diffuse_texture, uv));
    }
    
    float diffuse = max(-dot(normal, normalize(light_direction)), bias);
    texture_color *= diffuse;
    
    float conceal = 0.0;

    if (disable_fog == 0) {
        float distance = length(camera_position - world_frag_position);
        float d = max(distance - view_distance * fog_bias, 0.0);
        float exp = d / (view_distance * (1.0 - fog_bias));
        float value = pow(4.0, exp);
        conceal = clamp(exp * exp, 0.0, 1.0);
    }

    FragColor = vec4(vec3(mix(texture_color, fog_color, conceal)), 1.0);
}