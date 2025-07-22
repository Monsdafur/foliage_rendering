#pragma once

#include "glad/glad.h"
#include "glm/ext/vector_float3.hpp"
#include <vector>

struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
};

class Mesh {
  public:
    Mesh(std::vector<Vertex>& vertices);

    GLuint get_vertex_array_id() const;

    GLuint get_vertex_buffer_id() const;

    const std::vector<Vertex>& get_vertices() const;

  private:
    GLuint m_vertex_array;
    GLuint m_vertex_buffer;
    std::vector<Vertex> m_vertices;
};