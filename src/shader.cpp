#include "shader.hpp"
#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

Shader::Shader() { m_id = glCreateProgram(); }

Shader::~Shader() { glDeleteProgram(m_id); }

static bool
check_shader_compilation_error(GLuint shader,
                               const std::filesystem::path& file_path) {
    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char error_log[512];
        glGetShaderInfoLog(shader, 512, NULL, error_log);
        std::cout << "FILE: " << file_path << std::endl;
        std::cout << error_log << std::endl;
    }

    return success;
}

static bool check_shader_linking_error(GLuint shader_program,
                                       const std::filesystem::path& path) {
    int success = 0;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        char error_log[512];
        glGetProgramInfoLog(shader_program, 512, NULL, error_log);
        std::cout << "ERROR LINKING PROGRAM: " << path << std::endl;
        std::cout << error_log << std::endl;
    }

    return success;
}

static std::string fetch_shader_code(const std::filesystem::path& path) {
    std::fstream file(path);
    if (!file.is_open()) {
        std::cerr << "SHADER SOURCE FILE NOT FOUND: " << path << std::endl;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    std::string shader_code = buffer.str();
    return shader_code;
}

bool Shader::load_shader_from_path(const std::filesystem::path& shader_path,
                                   GLuint type) {
    // shader source
    std::string shader_code_string = fetch_shader_code(shader_path);
    const char* shader_code = shader_code_string.c_str();

    // load shader
    GLuint shader;
    shader = glCreateShader(type);
    glShaderSource(shader, 1, &shader_code, NULL);
    glCompileShader(shader);
    if (!check_shader_compilation_error(shader, shader_path)) {
        return false;
    }

    // link program
    glAttachShader(m_id, shader);
    glLinkProgram(m_id);
    if (!check_shader_linking_error(m_id, shader_path)) {
        return false;
    }

    glDeleteShader(shader);

    return true;
}

void Shader::set_uniform_int(const std::string& name, int value) {
    GLuint location = glGetUniformLocation(m_id, name.c_str());
    glUseProgram(m_id);
    glUniform1i(location, value);
    glUseProgram(0);
}

void Shader::set_uniform_float(const std::string& name, float value) {
    GLuint location = glGetUniformLocation(m_id, name.c_str());
    glUseProgram(m_id);
    glUniform1f(location, value);
    glUseProgram(0);
}

void Shader::set_uniform_vector2(const std::string& name,
                                 const glm::vec2& value) {
    GLuint location = glGetUniformLocation(m_id, name.c_str());
    glUseProgram(m_id);
    glUniform2f(location, value[0], value[1]);
    glUseProgram(0);
}

void Shader::set_uniform_vector3(const std::string& name,
                                 const glm::vec3& value) {
    GLuint location = glGetUniformLocation(m_id, name.c_str());
    glUseProgram(m_id);
    glUniform3f(location, value[0], value[1], value[2]);
    glUseProgram(0);
}

void Shader::set_uniform_vector4(const std::string& name,
                                 const glm::vec4& value) {
    GLuint location = glGetUniformLocation(m_id, name.c_str());
    glUseProgram(m_id);
    glUniform4f(location, value[0], value[1], value[2], value[3]);
    glUseProgram(0);
}

void Shader::set_uniform_matrix2(const std::string& name,
                                 const glm::mat2& value) {
    GLuint location = glGetUniformLocation(m_id, name.c_str());
    glUseProgram(m_id);
    glUniformMatrix2fv(location, 1, GL_FALSE, glm::value_ptr(value));
    glUseProgram(0);
}

void Shader::set_uniform_matrix3(const std::string& name,
                                 const glm::mat3& value) {
    GLuint location = glGetUniformLocation(m_id, name.c_str());
    glUseProgram(m_id);
    glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(value));
    glUseProgram(0);
}

void Shader::set_uniform_matrix4(const std::string& name,
                                 const glm::mat4& value) {
    GLuint location = glGetUniformLocation(m_id, name.c_str());
    glUseProgram(m_id);
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
    glUseProgram(0);
}

GLuint Shader::get_id() const { return m_id; };