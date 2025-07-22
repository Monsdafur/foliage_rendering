#include "shader.hpp"
#include "glad/glad.h"
#include "glm/gtc/type_ptr.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

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

static bool check_shader_linking_error(
    GLuint shader_program, const std::filesystem::path& vertex_shader_file_path,
    const std::filesystem::path& fragment_shader_file_path) {
    int success = 0;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        char error_log[512];
        glGetProgramInfoLog(shader_program, 512, NULL, error_log);
        std::cout << "ERROR LINKING PROGRAM: " << vertex_shader_file_path
                  << " AND " << fragment_shader_file_path << std::endl;
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

bool Shader::load_shader_from_path(
    const std::filesystem::path& vertex_shader_path,
    const std::filesystem::path& fragment_shader_path) {
    // fetch vertex shader source
    std::string vertex_shader_code_string =
        fetch_shader_code(vertex_shader_path);
    const char* vertex_shader_code = vertex_shader_code_string.c_str();

    // fetch fragment shader source
    std::string fragment_shader_code_string =
        fetch_shader_code(fragment_shader_path);
    const char* fragment_shader_code = fragment_shader_code_string.c_str();

    GLuint vertex_shader;
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_code, NULL);
    glCompileShader(vertex_shader);
    if (!check_shader_compilation_error(vertex_shader, vertex_shader_path)) {
        return false;
    }

    // fragment shader
    GLuint fragment_shader;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_code, NULL);
    glCompileShader(fragment_shader);
    if (!check_shader_compilation_error(fragment_shader,
                                        fragment_shader_path)) {
        return false;
    }

    // shader program
    m_id = glCreateProgram();
    glAttachShader(m_id, vertex_shader);
    glAttachShader(m_id, fragment_shader);
    glLinkProgram(m_id);
    if (!check_shader_linking_error(m_id, vertex_shader_path,
                                    fragment_shader_path)) {
        return false;
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

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
    glUniformMatrix2fv(location, 1, GL_TRUE, glm::value_ptr(value));
    glUseProgram(0);
}

void Shader::set_uniform_matrix3(const std::string& name,
                                 const glm::mat3& value) {
    GLuint location = glGetUniformLocation(m_id, name.c_str());
    glUseProgram(m_id);
    glUniformMatrix3fv(location, 1, GL_TRUE, glm::value_ptr(value));
    glUseProgram(0);
}

void Shader::set_uniform_matrix4(const std::string& name,
                                 const glm::mat4& value) {
    GLuint location = glGetUniformLocation(m_id, name.c_str());
    glUseProgram(m_id);
    glUniformMatrix4fv(location, 1, GL_TRUE, glm::value_ptr(value));
    glUseProgram(0);
}

GLuint Shader::get_id() const { return m_id; };