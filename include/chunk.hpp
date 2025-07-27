#pragma once

#include "mesh.hpp"
#include "renderer.hpp"
#include "shader.hpp"

struct GrassBuffer {
    glm::mat4 transform;
    glm::mat4 sway;
};

class Chunk {
  public:
    Chunk(const Mesh& grass_mesh, Shader& generator, glm::ivec3 position,
          int grass_per_unit, int size);

    void update(Shader& flow_field, Shader& displacement, float wind_direction,
                float time);

    void render(Renderer& renderer, Shader& standard, Shader& gpu_instancing);

    void frustum_test(const Camera& camera);

    Mesh m_ground;

    static int grass_count;

  private:
    glm::ivec3 m_position;

    const Mesh& m_grass_mesh;

    int m_size;
    int m_grass_count;
    int m_grass_per_unit;

    bool m_cull = false;

    ShaderBuffer<GrassBuffer> m_grass_buffer;
    Texture m_height_map;
    Texture m_noise_map;

    glm::vec3 m_min;
    glm::vec3 m_max;
};