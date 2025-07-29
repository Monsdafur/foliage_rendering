#include "chunk.hpp"
#include "PerlinNoise.hpp"
#include "glad/glad.h"
#include "glm/ext/vector_int2.hpp"
#include "glm/geometric.hpp"
#include "glm/matrix.hpp"
#include "mesh.hpp"
#include "utility.hpp"
#include <cstdint>

int Chunk::grass_count = 0;

Chunk::Chunk(const Mesh& grass_mesh, Shader& generator, glm::ivec3 position,
             int grass_per_unit, int size, float terrain_height,
             float terrain_scale, uint64_t seed)
    : m_grass_mesh(grass_mesh), m_size(size), m_grass_per_unit(grass_per_unit) {
    m_grass_count = m_size * m_size * grass_per_unit * grass_per_unit;
    grass_count += m_grass_count;
    m_min = glm::vec3(position) * (float)size;
    m_max = m_min + glm::vec3(m_size, 0.0f, m_size);

    uint8_t bytes[(m_size + 1) * (m_size + 1) * 3];
    std::vector<Vertex> ground_vertices;
    std::vector<Vertex> ground_vertices_low_poly;
    std::vector<int> ground_indices;
    std::vector<int> ground_indices_low_poly;

    const siv::PerlinNoise::seed_type seed_type = seed;
    const siv::PerlinNoise perlin{seed_type};

    m_min.y = 10000.0f;
    m_max.y = -10000.0f;

    for (int x = 0; x <= m_size; ++x) {
        for (int z = 0; z <= m_size; ++z) {
            glm::vec3 position = m_min + glm::vec3(x, 0.0f, z);
            position.y = perlin.octave2D_01(position.x * terrain_scale,
                                            position.z * terrain_scale, 14) *
                         terrain_height;

            if (x < m_size && z < m_size) {
                int s = m_size + 1;
                ground_indices.push_back(x + s * z);
                ground_indices.push_back((x + 1) + s * z);
                ground_indices.push_back(x + s * (z + 1));

                ground_indices.push_back((x + 1) + s * z);
                ground_indices.push_back((x + 1) + s * (z + 1));
                ground_indices.push_back(x + s * (z + 1));
            }

            glm::vec3 p0 = position + glm::vec3(1.0f, 0.0f, 0.0f);
            glm::vec3 p1 = position + glm::vec3(-1.0f, 0.0f, 0.0f);
            glm::vec3 p2 = position + glm::vec3(0.0f, 0.0f, 1.0f);
            glm::vec3 p3 = position + glm::vec3(0.0f, 0.0f, -1.0f);

            p0.y = perlin.octave2D_01(p0.x * terrain_scale,
                                      p0.z * terrain_scale, 14) *
                   terrain_height;
            p1.y = perlin.octave2D_01(p1.x * terrain_scale,
                                      p1.z * terrain_scale, 14) *
                   terrain_height;
            p2.y = perlin.octave2D_01(p2.x * terrain_scale,
                                      p2.z * terrain_scale, 14) *
                   terrain_height;
            p3.y = perlin.octave2D_01(p3.x * terrain_scale,
                                      p3.z * terrain_scale, 14) *
                   terrain_height;

            float h0 = p0.y - p1.y;
            float h1 = p2.y - p3.y;
            glm::vec3 n = glm::normalize(glm::vec3(-h0, -h1, -1.0f));

            ground_vertices.push_back(
                {position, {0.0f, 0.0f}, n, {0.06f, 0.12f, 0.0f}});

            m_min.y = fmin(m_min.y, position.y);
            m_max.y = fmax(m_max.y, position.y);

            int ci = (x + (m_size + 1) * z) * 3;
            float d = position.y / terrain_height;
            bytes[ci + 0] = (uint8_t)(d * 255.0f);
            bytes[ci + 1] = (uint8_t)(d * 255.0f);
            bytes[ci + 2] = (uint8_t)(d * 255.0f);
        }
    }
    m_max.y += 4.0f;

    for (int x = 0; x <= m_size / 4; ++x) {
        for (int z = 0; z <= m_size / 4; ++z) {
            int i = (x * 4 + (m_size + 1) * z * 4);
            ground_vertices_low_poly.push_back(ground_vertices[i]);
            if (x < m_size / 4 && z < m_size / 4) {
                int s = m_size / 4 + 1;
                ground_indices_low_poly.push_back(x + s * (z + 1));
                ground_indices_low_poly.push_back((x + 1) + s * z);
                ground_indices_low_poly.push_back(x + s * z);

                ground_indices_low_poly.push_back(x + s * (z + 1));
                ground_indices_low_poly.push_back((x + 1) + s * (z + 1));
                ground_indices_low_poly.push_back((x + 1) + s * z);
            }
        }
    }

    // printf("lx: %.1f, ly: %.1f, lz: %.1f\n", m_min.x, m_min.y, m_min.z);
    // printf("hx: %.1f, hy: %.1f, hz: %.1f\n", m_max.x, m_max.y, m_max.z);

    m_height_map.load_texture_from_byte(bytes, GL_UNSIGNED_BYTE,
                                        glm::ivec2(m_size + 1, m_size + 1),
                                        GL_RGB, GL_RGB);
    m_height_map.set_filter_mode(GL_LINEAR);
    m_height_map.set_wrap_mode(GL_CLAMP_TO_EDGE);
    m_ground.set(ground_vertices, ground_indices);
    m_ground_low_poly.set(ground_vertices_low_poly, ground_indices_low_poly);

    std::vector<GrassBuffer> grass(m_grass_count);
    m_grass_buffer.load_data(grass);

    generator.set_uniform_int("width", m_size * m_grass_per_unit);
    generator.set_uniform_int("height", m_size * m_grass_per_unit);
    generator.set_uniform_vector3("lower_bound", m_min);
    generator.set_uniform_vector3("upper_bound", m_max);
    generator.set_uniform_float("spacing", 1.0f / m_grass_per_unit);
    generator.set_uniform_float("terrain_scale", terrain_height);
    generator.set_uniform_texture("height_map", m_height_map, 0);

    generator.set_buffer(m_grass_buffer, 0);
    generator.dispatch(
        glm::ivec3(m_size * m_grass_per_unit, m_size * m_grass_per_unit, 1));
    gl_check_error();
    generator.flush_textures();

    m_noise_map.load_texture_from_byte(0, GL_FLOAT,
                                       glm::ivec2(m_size * m_grass_per_unit),
                                       GL_RGBA32F, GL_RGBA);
    m_noise_map.set_filter_mode(GL_LINEAR);
}

void Chunk::update(Shader& flow_field, Shader& displacement,
                   float wind_direction, float time) {
    if (m_cull || m_far) {
        return;
    }

    flow_field.set_uniform_float("shift", time);
    flow_field.set_uniform_vector2("offset", glm::vec2(m_min.x, m_min.z) *
                                                 (float)(m_grass_per_unit));

    flow_field.dispatch_texture(m_noise_map,
                                glm::ivec3(m_noise_map.get_size(), 1));

    displacement.set_uniform_int("width", m_size * m_grass_per_unit);
    displacement.set_uniform_int("height", m_size * m_grass_per_unit);
    displacement.set_uniform_texture("noise_map", m_noise_map, 0);
    displacement.set_buffer(m_grass_buffer, 0);
    displacement.dispatch(
        glm::ivec3(m_size * m_grass_per_unit, m_size * m_grass_per_unit, 1));
    displacement.flush_textures();
}

void Chunk::render(Renderer& renderer, Shader& standard, Shader& gpu_instancing,
                   bool debug) {
    if (m_cull) {
        return;
    }

    renderer.draw(m_far ? m_ground_low_poly : m_ground, glm::mat4(1.0f),
                  standard);
    renderer.draw_instances(m_grass_mesh, m_grass_buffer, gpu_instancing,
                            m_grass_count);
}

void Chunk::frustum_test(const Camera& camera) {
    glm::vec3 d = camera.get_position() - (m_min + (m_max - m_min) * 0.5f);
    float r = camera.get_far_clip_plane() * 0.85f;
    m_far = glm::dot(d, d) > r * r;

    glm::vec4 planes[6];

    glm::mat4 mat = glm::transpose(camera.get_matrix());

    planes[0] = mat[3] + mat[0];
    planes[1] = mat[3] - mat[0];
    planes[2] = mat[3] + mat[1];
    planes[3] = mat[3] - mat[1];
    planes[4] = mat[3] + mat[2];
    planes[5] = mat[3] - mat[2];

    for (int i = 0; i < 6; ++i) {
        float length = glm::length(glm::vec3(planes[i]));
        planes[i] /= length;
    }

    m_cull = false;
    for (int i = 0; i < 6; ++i) {
        glm::vec3 n;
        n.x = (planes[i].x <= 0.0f) ? m_min.x : m_max.x;
        n.y = (planes[i].y <= 0.0f) ? m_min.y : m_max.y;
        n.z = (planes[i].z <= 0.0f) ? m_min.z : m_max.z;

        float distance = glm::dot(glm::vec3(planes[i]), n) + planes[i].w;

        if (distance < 0.0f) {
            m_cull = true;
            break;
        }
    }
}