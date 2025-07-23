#pragma once

#include "glm/glm.hpp"
#include "mesh.hpp"
#include "shader.hpp"

class Camera {
  public:
    Camera() = default;

    Camera(const glm::vec3& position, float fov, float aspect_ratio);

    glm::vec3 get_position() const;

    void set_position(const glm::vec3& position);

    void look_at(const glm::vec3 position);

    glm::mat4 get_matrix() const;

  private:
    glm::vec3 m_position;
    glm::vec3 m_direction;
    glm::vec3 m_up;
    glm::mat4 m_projection;
};

class Renderer {
  public:
    Renderer();

    void draw(const Mesh& mesh, const glm::mat4& transform, Shader& shader,
              GLuint mode = GL_TRIANGLES);

    void set_camera(const Camera& camera);

  private:
    glm::mat4 m_camera_matrix;
    static bool m_glad_initialized;
};