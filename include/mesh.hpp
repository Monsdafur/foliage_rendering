#pragma once

#include "glad/glad.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "texture.hpp"
#include <vector>

struct Vertex {
    glm::vec3 position;
    glm::vec2 uv;
    glm::vec3 normal;
    glm::vec3 color;
};

class Mesh {
  public:
    Mesh(std::vector<Vertex>& vertices, std::vector<int>& indices);

    ~Mesh();

    GLuint get_vertex_array_id() const;

    GLuint get_vertex_buffer_id() const;

    const std::vector<Vertex>& get_vertices() const;

    const std::vector<int>& get_indices() const;

    glm::mat4 get_transform_matrix() const;

    std::shared_ptr<const Texture> get_texture() const;

    void set_texture(std::shared_ptr<const Texture> texture);

  private:
    GLuint m_vertex_array;
    GLuint m_vertex_buffer;
    GLuint m_element_buffer;

    std::vector<Vertex> m_vertices;
    std::vector<int> m_indices;

    std::shared_ptr<const Texture> m_texture;
};