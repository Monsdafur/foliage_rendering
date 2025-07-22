#pragma once

#include "glad/glad.h"
#include "glm/fwd.hpp"
#include <filesystem>

class Shader {
  public:
    Shader() = default;

    ~Shader();

    bool
    load_shader_from_path(const std::filesystem::path& vertex_shader_path,
                          const std::filesystem::path& fragment_shader_path);

    void set_uniform_int(const std::string& name, int value);

    void set_uniform_float(const std::string& name, float value);

    void set_uniform_vector2(const std::string& name, const glm::vec2& value);

    void set_uniform_vector3(const std::string& name, const glm::vec3& value);

    void set_uniform_vector4(const std::string& name, const glm::vec4& value);

    void set_uniform_matrix2(const std::string& name, const glm::mat2& value);

    void set_uniform_matrix3(const std::string& name, const glm::mat3& value);

    void set_uniform_matrix4(const std::string& name, const glm::mat4& value);

    GLuint get_id() const;

  private:
    GLuint m_id;

    std::filesystem::path m_vertex_shader_path;
    std::filesystem::path m_fragment_shader_path;
};