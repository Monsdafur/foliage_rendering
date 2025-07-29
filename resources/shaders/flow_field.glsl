#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D image_output;

uniform float shift;
uniform vec2 wind_direction;
uniform vec2 offset;

float hash(float p) { 
    p = fract(p * 0.011); 
    p *= p + 7.5; p *= p + p; 
    return fract(p); 
}

float hash(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.13); 
    p3 += dot(p3, p3.yzx + 3.333); 
    return fract((p3.x + p3.y) * p3.z); 
}

float noise(vec2 x) {
    vec2 i = floor(x);
    vec2 f = fract(x);

	float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);
	return mix(a, b, u.x) + (c - a) * u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

void main() {
    ivec2 texel_coord = ivec2(gl_GlobalInvocationID.xy);
    float theta = 0.25;
    float c = wind_direction.x;
    float s = wind_direction.y;
    vec2 position = vec2(texel_coord) + offset;
	float x = c * position.x + s * position.y;
	float y = -s * position.x + c * position.y;

    float h = sin(x * 0.03 + shift) * 0.2 + 0.3;
    h += noise(((vec2(texel_coord) * 0.1) + wind_direction * shift)) * 0.125 - 0.0625;
    h += noise(((vec2(texel_coord) * 0.06) + wind_direction * shift)) * 0.3 - 0.15;
    h = clamp(h, 0.0, 1.0);
	
    imageStore(image_output, texel_coord, vec4(h, h, h, 1.0));
}