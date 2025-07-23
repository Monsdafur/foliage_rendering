#version 430 core
out vec4 FragColor;

in vec3 color;
in vec3 normal;

uniform vec3 light_direction;
uniform float bias;
uniform float near;
uniform float far;
uniform float fog_bias;
uniform vec3 fog_color;

float linearize_depth(float depth) 
{
    float z = depth * 2.0f - 1.0f; 
    return (2.0f * near * far) / (far + near - z * (far - near));	
}

void main()
{
    vec3 final_color = color;
    float diffuse = max(-dot(normal, normalize(light_direction)), bias);
    final_color *= diffuse;
    
    float depth = linearize_depth(gl_FragCoord.z) / far;
    depth = max(depth - fog_bias, 0.0f);

    FragColor = vec4(vec3(mix(final_color, fog_color, depth)), 1.0f);
}