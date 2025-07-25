#version 430 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D image_output;

uniform float shift;
uniform vec2 wind_direction;

float hash(float n) { return fract(sin(n) * 1e4); }
float hash(vec2 p) { return fract(1e4 * sin(17.0 * p.x + p.y * 0.1) * (0.1 + abs(sin(p.y * 13.0 + p.x)))); }

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
    vec2 position = (vec2(texel_coord)) + wind_direction * shift;
	float x = c * position.x + s * position.y;
	float y = -s * position.x + c * position.y;

    float h = noise(vec2(x, y) * 0.25);
    h += sin(x * 0.1 + shift) * 0.25;
    h = clamp(h, 0.0, 1.0);
	
    imageStore(image_output, texel_coord, vec4(h, h, h, 1.0));
}