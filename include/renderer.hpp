#pragma once

#include "glm/glm.hpp"
#include "mesh.hpp"
#include "shader.hpp"

class Camera {
  public:
    Camera() = default;

    Camera(const glm::vec3& position, float fov, float aspect_ratio, float near,
           float far);

    glm::vec3 get_position() const;

    void set_position(const glm::vec3& position);

    void look_at(const glm::vec3 position);

    glm::mat4 get_matrix() const;

  private:
    glm::vec3 m_position;
    glm::vec3 m_direction;
    glm::vec3 m_up;
    glm::mat4 m_projection;

    float m_near;
    float m_far;
};

template <typename T> class ShaderBuffer {
  public:
    ShaderBuffer(const std::vector<T>& data);

    ~ShaderBuffer();

    GLuint get_id() const;

  private:
    GLuint m_shader_buffer_object;
};

template <typename T>
ShaderBuffer<T>::ShaderBuffer(const std::vector<T>& data) {
    glGenBuffers(1, &m_shader_buffer_object);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_shader_buffer_object);
    glBufferData(GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(T), data.data(),
                 GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

template <typename T> ShaderBuffer<T>::~ShaderBuffer() {
    glDeleteBuffers(1, &m_shader_buffer_object);
}

template <typename T> GLuint ShaderBuffer<T>::get_id() const {
    return m_shader_buffer_object;
}

class Renderer {
  public:
    Renderer();

    void draw(const Mesh& mesh, const glm::mat4& transform, Shader& shader,
              GLuint mode = GL_TRIANGLES);

    template <typename T>
    void draw_instances(const Mesh& mesh, const ShaderBuffer<T>& shader_buffer,
                        Shader& shader, int count, GLuint mode = GL_TRIANGLES);

    void set_camera(const Camera& camera);

  private:
    glm::mat4 m_camera_matrix;
    static bool m_glad_initialized;
};

template <typename T>
void Renderer::draw_instances(const Mesh& mesh,
                              const ShaderBuffer<T>& shader_buffer,
                              Shader& shader, int count, GLuint mode) {

    shader.set_uniform_matrix4("projection", m_camera_matrix);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, shader_buffer.get_id());
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, shader_buffer.get_id());

    glUseProgram(shader.get_id());

    glBindVertexArray(mesh.get_vertex_array_id());

    glDrawElementsInstanced(mode, mesh.get_indices().size(), GL_UNSIGNED_INT,
                            mesh.get_indices().data(), count);

    glBindVertexArray(0);

    glUseProgram(0);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, 0);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}