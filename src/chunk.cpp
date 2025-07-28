#include "chunk.hpp"
#include "PerlinNoise.hpp"
#include "glad/glad.h"
#include "glm/ext/vector_int2.hpp"
#include "glm/geometric.hpp"
#include "glm/matrix.hpp"
#include "mesh.hpp"
#include "utility.hpp"

int Chunk::grass_count = 0;

Chunk::Chunk(const Mesh& grass_mesh, const Mesh& grass_mesh_low_poly,
             Shader& generator, glm::ivec3 position, int grass_per_unit,
             int size)
    : m_grass_mesh(grass_mesh), m_grass_mesh_low_poly(grass_mesh_low_poly),
      m_size(size), m_grass_per_unit(grass_per_unit) {
    m_grass_count = m_size * m_size * grass_per_unit * grass_per_unit;
    grass_count += m_grass_count;
    m_min = glm::vec3(position) * (float)size;
    m_max = m_min + glm::vec3(m_size, 0.0f, m_size);

    float scale = 20.0f;
    int indices = 0;
    uint8_t bytes[m_size * m_size * 3];
    std::vector<Vertex> ground_vertices;
    std::vector<int> ground_indices;

    const siv::PerlinNoise::seed_type seed = 123456u;

    const siv::PerlinNoise perlin{seed};

    m_min.y = 100.0f;
    m_max.y = -100.0f;
    for (int x = 0; x < m_size; ++x) {
        for (int y = 0; y < m_size; ++y) {
            int x0 = m_min.x + x;
            int y0 = m_min.z + y;
            int x1 = x0 + 1;
            int y1 = y0 + 1;
            // float height = sin(x * 0.25f);
            float h0 =
                perlin.octave2D_01((x1 * 0.01f), (y1 * 0.01f), 4) * scale;
            float h1 =
                perlin.octave2D_01((x1 * 0.01f), (y0 * 0.01f), 4) * scale;
            float h2 =
                perlin.octave2D_01((x0 * 0.01f), (y0 * 0.01f), 4) * scale;
            float h3 =
                perlin.octave2D_01((x0 * 0.01f), (y1 * 0.01f), 4) * scale;
            glm::vec3 v0(x1, h0, y1);
            glm::vec3 v1(x1, h1, y0);
            glm::vec3 v2(x0, h2, y0);
            glm::vec3 v3(x0, h3, y1);
            glm::vec3 n0 = glm::normalize(glm::cross(v1 - v0, v3 - v0));
            glm::vec3 n1 = glm::normalize(glm::cross(v2 - v1, v0 - v1));
            glm::vec3 n2 = glm::normalize(glm::cross(v3 - v2, v1 - v2));
            glm::vec3 n3 = glm::normalize(glm::cross(v0 - v3, v2 - v3));
            glm::vec3 color(0.07f, 0.12f, 0.0f);
            ground_vertices.push_back({v0, {0.0f, 0.0f}, n0, color});
            ground_vertices.push_back({v1, {0.0f, 0.0f}, n1, color});
            ground_vertices.push_back({v2, {0.0f, 0.0f}, n2, color});
            ground_vertices.push_back({v3, {0.0f, 0.0f}, n3, color});
            ground_indices.push_back(indices);
            ground_indices.push_back(indices + 1);
            ground_indices.push_back(indices + 2);
            ground_indices.push_back(indices);
            ground_indices.push_back(indices + 2);
            ground_indices.push_back(indices + 3);
            indices += 4;

            int color_index = (x + m_size * y) * 3;
            bytes[color_index] = (h2 / scale) * 255.0f;
            bytes[color_index + 1] = (h2 / scale) * 255.0f;
            bytes[color_index + 2] = (h2 / scale) * 255.0f;

            m_min.y = fmin(fmin(fmin(fmin(m_min.y, h0), h1), h2), h3);
            m_max.y = fmax(fmax(fmax(fmax(m_max.y, h0), h1), h2), h3);
        }
    }
    m_max.y += 4.0f;

    // printf("lx: %.1f, ly: %.1f, lz: %.1f\n", m_min.x, m_min.y, m_min.z);
    // printf("hx: %.1f, hy: %.1f, hz: %.1f\n", m_max.x, m_max.y, m_max.z);

    m_height_map.load_texture_from_byte(
        bytes, GL_UNSIGNED_BYTE, glm::ivec2(m_size, m_size), GL_RGB, GL_RGB);
    m_height_map.set_filter_mode(GL_LINEAR);
    m_height_map.set_wrap_mode(GL_CLAMP_TO_EDGE);
    m_ground.set(ground_vertices, ground_indices);

    std::vector<GrassBuffer> grass(m_grass_count);
    m_grass_buffer.load_data(grass);

    generator.set_uniform_int("width", m_size * m_grass_per_unit);
    generator.set_uniform_int("height", m_size * m_grass_per_unit);
    generator.set_uniform_vector3("lower_bound", m_min);
    generator.set_uniform_vector3("upper_bound", m_max);
    generator.set_uniform_float("spacing", 1.0f / m_grass_per_unit);
    generator.set_uniform_float("terrain_scale", scale);
    generator.set_uniform_texture("height_map", m_height_map, 0);

    generator.set_buffer(m_grass_buffer, 0);
    generator.dispatch(
        glm::ivec3(m_size * m_grass_per_unit, m_size * m_grass_per_unit, 1));
    generator.flush_textures();

    m_noise_map.load_texture_from_byte(0, GL_FLOAT,
                                       glm::ivec2(m_size * m_grass_per_unit),
                                       GL_RGBA32F, GL_RGBA);
    m_noise_map.set_filter_mode(GL_LINEAR);
}

void Chunk::update(Shader& flow_field, Shader& displacement,
                   float wind_direction, float time) {
    if (m_cull) {
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
    gl_check_error();
    displacement.flush_textures();
}

void Chunk::render(Renderer& renderer, Shader& standard, Shader& gpu_instancing,
                   bool debug) {
    if (m_cull) {
        return;
    }

    renderer.draw(m_ground, glm::mat4(1.0f), standard);
    if (0) {
        if (!m_far) {
            renderer.draw_instances(m_grass_mesh, m_grass_buffer,
                                    gpu_instancing, m_grass_count);
        } else {
            renderer.draw_instances(m_grass_mesh_low_poly, m_grass_buffer,
                                    gpu_instancing, m_grass_count);
        }
    }
}

void Chunk::frustum_test(const Camera& camera) {
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

    glm::vec3 d = camera.get_position() - (m_min + (m_max - m_min) * 0.5f);
    m_far = glm::dot(d, d) > 144.0f * 144.0f;
}