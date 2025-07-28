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
    Chunk(const Mesh& grass_mesh, const Mesh& grass_mesh_low_poly,
          Shader& generator, glm::ivec3 position, int grass_per_unit, int size);

    void update(Shader& flow_field, Shader& displacement, float wind_direction,
                float time);

    void render(Renderer& renderer, Shader& standard, Shader& gpu_instancing,
                bool debug = false);

    void frustum_test(const Camera& camera);

    static int grass_count;

  private:
    const Mesh& m_grass_mesh;
    const Mesh& m_grass_mesh_low_poly;
    Mesh m_ground;
    Mesh m_ground_low_poly;

    int m_size;
    int m_grass_count;
    int m_grass_per_unit;

    bool m_cull = false;
    bool m_far = false;

    ShaderBuffer<GrassBuffer> m_grass_buffer;
    Texture m_height_map;
    Texture m_noise_map;

    glm::vec3 m_min;
    glm::vec3 m_max;
};