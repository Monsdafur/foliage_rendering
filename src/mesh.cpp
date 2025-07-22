#include "mesh.hpp"

Mesh::Mesh(std::vector<Vertex>& vertices) : m_vertices(std::move(vertices)) {
    glGenVertexArrays(1, &m_vertex_array);
    glBindVertexArray(m_vertex_array);

    glGenBuffers(1, &m_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, 0, sizeof(Vertex),
                          (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(1);

    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex),
                 m_vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

GLuint Mesh::get_vertex_array_id() const { return m_vertex_array; }

GLuint Mesh::get_vertex_buffer_id() const { return m_vertex_buffer; }

const std::vector<Vertex>& Mesh::get_vertices() const { return m_vertices; }