#version 430 core

layout(local_size_x = 1, local_size_y = 1) in;

struct GrassBuffer {
    mat4 transform;
    mat4 sway;
};

layout(std430, binding = 0) buffer BufferData {
    GrassBuffer grass_buffer[];
};

uniform int width;
uniform int height;
uniform vec3 start_position;
uniform float spacing;

float random_range(vec2 seed, float low, float high) {
    float r = fract(sin(dot(seed, vec2(12.9898, 78.233))) * 43758.5453);
    return mix(low, high, r);
}

mat4 translate(vec3 t) {
    return mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, 1.0, 0.0, 0.0,
        0.0, 0.0, 1.0, 0.0,
        t.x, t.y, t.z, 1.0
    );
}

mat4 scale(vec3 s) {
    return mat4(
        s.x, 0.0, 0.0, 0.0,
        0.0, s.y, 0.0, 0.0,
        0.0, 0.0, s.z, 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

mat4 rotation_x(float r) {
    float c = cos(r);
    float s = sin(r);
    return mat4(
        1.0, 0.0, 0.0, 0.0,
        0.0, c   , s , 0.0,
        0.0, -s  , c , 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

mat4 rotation_y(float r) {
    float c = cos(r);
    float s = sin(r);
    return mat4(
        c   , 0.0, -s  , 0.0,
        0.0, 1.0, 0.0, 0.0,
        s   , 0.0, c   , 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

void main() {
    uvec2 id = gl_GlobalInvocationID.xy;
    if (id.x < width && id.y < height) {
        int index = int(id.y) * width + int(id.x);

        float offset_theta = random_range(id, 0.0, 3.14159 * 2.0);
        vec3 offset;
        offset.x = cos(offset_theta);
        offset.z = sin(offset_theta);
        offset *= spacing;

        vec3 position = start_position + vec3(float(id.x), 0.0, float(id.y)) * spacing + offset;
        mat4 rotation = rotation_x(random_range(id, -0.01, 0.01)) *
                        rotation_y(random_range(id, 0.0, 3.14159 * 2.0));

        grass_buffer[index].transform = translate(position) * 
                                        rotation *
                                        scale(vec3(1.0, random_range(id, 1.0, 4.0), 1.0));
        grass_buffer[index].sway[0][0] = random_range(id, 0.1, 1.0);
        grass_buffer[index].sway[3][0] = position.x;
        grass_buffer[index].sway[3][1] = position.z;
    }
}