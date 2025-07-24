#pragma once

#include "glad/glad.h"
#include "glm/glm.hpp"
#include <filesystem>

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
                 GL_DYNAMIC_COPY);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

template <typename T> ShaderBuffer<T>::~ShaderBuffer() {
    glDeleteBuffers(1, &m_shader_buffer_object);
}

template <typename T> GLuint ShaderBuffer<T>::get_id() const {
    return m_shader_buffer_object;
}

class Shader {
  public:
    Shader();

    ~Shader();

    bool load_shader_from_path(const std::filesystem::path& shader_path,
                               GLuint type);

    void set_uniform_int(const std::string& name, int value);

    void set_uniform_float(const std::string& name, float value);

    void set_uniform_vector2(const std::string& name, const glm::vec2& value);

    void set_uniform_vector3(const std::string& name, const glm::vec3& value);

    void set_uniform_vector4(const std::string& name, const glm::vec4& value);

    void set_uniform_matrix2(const std::string& name, const glm::mat2& value);

    void set_uniform_matrix3(const std::string& name, const glm::mat3& value);

    void set_uniform_matrix4(const std::string& name, const glm::mat4& value);

    GLuint get_id() const;

    template <typename T>
    void dispatch_buffer_data(ShaderBuffer<T>& buffer,
                              const glm::ivec3& work_groups);

  private:
    GLuint m_id;

    std::filesystem::path m_vertex_shader_path;
    std::filesystem::path m_fragment_shader_path;
};

template <typename T>
void Shader::dispatch_buffer_data(ShaderBuffer<T>& buffer,
                                  const glm::ivec3& work_groups) {
    glUseProgram(m_id);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer.get_id());

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, buffer.get_id());
    glDispatchCompute(work_groups.x, work_groups.y, work_groups.z);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glUseProgram(0);
}