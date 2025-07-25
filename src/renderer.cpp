#include "renderer.hpp"
#include "GLFW/glfw3.h"
#include "glad/glad.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/geometric.hpp"
#include <iostream>

Camera::Camera(const glm::vec3& position, float fov, float aspect_ratio,
               float near, float far)
    : m_position(position), m_near(near), m_far(far) {
    m_projection = glm::perspective(fov, aspect_ratio, m_near, m_far);
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

float Camera::get_near_clip_plane() const { return m_near; }

float Camera::get_far_clip_plane() const { return m_far; }

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
    shader.set_uniform_int("has_texture", mesh.get_texture() != nullptr);
    if (mesh.get_texture()) {
        shader.set_uniform_texture("diffuse_texture", *mesh.get_texture(), 0);
    } else {
        glUseProgram(shader.get_id());
    }
    glBindVertexArray(mesh.get_vertex_array_id());

    glDrawElements(mode, mesh.get_indices().size(), GL_UNSIGNED_INT,
                   mesh.get_indices().data());

    glBindVertexArray(0);
    if (mesh.get_texture()) {
        shader.flush_textures();
    }
}