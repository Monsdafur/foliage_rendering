#include "renderer.hpp"
#include "GLFW/glfw3.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/geometric.hpp"
#include <iostream>

Camera::Camera(const glm::vec3& position, float fov, float aspect_ratio,
               float near, float far)
    : m_position(position) {
    m_projection = glm::perspective(fov, aspect_ratio, near, far);
    m_direction = glm::vec3(0.0f, 0.0f, -1.0f);
    m_up = glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::vec3 Camera::get_position() const { return m_position; }

void Camera::set_position(const glm::vec3& position) { m_position = position; }

void Camera::look_at(const glm::vec3 position) {
    m_direction = glm::normalize(position - m_position);
    m_up = glm::cross(glm::cross(m_direction, glm::vec3(0.0f, 1.0f, 0.0f)),
                      m_direction);
}

glm::mat4 Camera::get_matrix() const {
    return m_projection *
           glm::lookAt(m_position, m_position + m_direction, m_up);
}

ShaderBuffer::ShaderBuffer(const std::vector<BufferData>& data) {
    glGenBuffers(1, &m_shader_buffer_object);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_shader_buffer_object);
    glBufferData(GL_SHADER_STORAGE_BUFFER, data.size() * sizeof(BufferData),
                 data.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

GLuint ShaderBuffer::get_id() const { return m_shader_buffer_object; }

bool Renderer::m_glad_initialized = false;

Renderer::Renderer() {
    if (!m_glad_initialized) {
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            std::cout << "Failed to initialize GLAD" << std::endl;
        }
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
    }
    m_glad_initialized = true;
}

void Renderer::set_camera(const Camera& camera) {
    m_camera_matrix = camera.get_matrix();
}

void Renderer::draw(const Mesh& mesh, const glm::mat4& transform,
                    Shader& shader, GLuint mode) {
    shader.set_uniform_matrix4("projection", m_camera_matrix);
    shader.set_uniform_matrix4("transform", transform);
    glUseProgram(shader.get_id());
    glBindVertexArray(mesh.get_vertex_array_id());
    glDrawElements(mode, mesh.get_indices().size(), GL_UNSIGNED_INT,
                   mesh.get_indices().data());
    glBindVertexArray(0);
    glUseProgram(0);
}

void Renderer::draw_instances(const Mesh& mesh,
                              const ShaderBuffer& shader_buffer, Shader& shader,
                              int count, GLuint mode) {

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