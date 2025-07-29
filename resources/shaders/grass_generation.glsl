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
uniform vec3 lower_bound;
uniform vec3 upper_bound;
uniform float spacing;
uniform sampler2D height_map;
uniform float terrain_scale;

float random(vec2 seed) {
    return fract(sin(dot(seed.xy, vec2(12.9898,78.233))) * 43758.5453123);
}

float random_range(vec2 seed, float low, float high) {
    return low + random(seed) * (high - low);
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
        c  , 0.0, -s , 0.0,
        0.0, 1.0, 0.0, 0.0,
        s  , 0.0, c  , 0.0,
        0.0, 0.0, 0.0, 1.0
    );
}

void main() {
    uvec2 id = gl_GlobalInvocationID.xy;
    vec3 uniform_position = lower_bound + vec3(id.x, 0.0, id.y) * spacing;
    vec2 seed = vec2(id) + vec2(lower_bound.xz);
    if (id.x < width && id.y < height) {
        int index = int(id.y) * width + int(id.x);

        vec3 offset;
        offset.x = random_range(seed, 0.0, spacing);
        offset.z = random_range(seed + vec2(1.0), 0.0, spacing);

        vec3 position = uniform_position + offset;
        mat4 rotation = rotation_y(random_range(seed, 0.0, 3.14159 * 2.0));

        vec2 uv = vec2((position.x - lower_bound.x) / (upper_bound.x - lower_bound.x),
                       (position.z - lower_bound.z) / (upper_bound.z - lower_bound.z));

        uv.x = clamp(uv.x, 0.0, 1.0);
        uv.y = clamp(uv.y, 0.0, 1.0);
        float height = random_range(seed, 1.0, 4.0);

        position.y = texture(height_map, uv).r * terrain_scale;
        grass_buffer[index].transform = translate(position) * 
                                        rotation *
                                        scale(vec3(1.0, height, 1.0));
        grass_buffer[index].sway[0][0] = 1.0;
        grass_buffer[index].sway[3][0] = uv.x;
        grass_buffer[index].sway[3][1] = uv.y;
        grass_buffer[index].sway[3][2] = height;
    }
}