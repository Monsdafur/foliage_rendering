#include "mesh.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<int>& indices)
    : m_vertices(std::move(vertices)), m_indices(std::move(indices)) {
    glGenVertexArrays(1, &m_vertex_array);
    glBindVertexArray(m_vertex_array);

    // vertex buffer
    glGenBuffers(1, &m_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, m_vertex_buffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, 0, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, 0, sizeof(Vertex),
                          (void*)sizeof(glm::vec3));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, 0, sizeof(Vertex),
                          (void*)(2 * sizeof(glm::vec3)));
    glEnableVertexAttribArray(2);

    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex),
                 m_vertices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    // element buffer
    glGenBuffers(1, &m_element_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_element_buffer);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(int),
                 m_indices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glBindVertexArray(0);
}

Mesh::~Mesh() {
    glDeleteBuffers(1, &m_vertex_buffer);
    glDeleteBuffers(1, &m_element_buffer);
    glDeleteVertexArrays(1, &m_vertex_array);
}

GLuint Mesh::get_vertex_array_id() const { return m_vertex_array; }

GLuint Mesh::get_vertex_buffer_id() const { return m_vertex_buffer; }

const std::vector<Vertex>& Mesh::get_vertices() const { return m_vertices; }

const std::vector<int>& Mesh::get_indices() const { return m_indices; }