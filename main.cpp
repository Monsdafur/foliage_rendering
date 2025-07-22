#include "external/glfw/src/internal.h"
#include "glad/glad.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/glm.hpp"
#include "include/shader.hpp"
#include "include/window.hpp"
#include "utility.hpp"
#include <GL/gl.h>
#include <cstdint>
#include <iostream>
#include <memory>

int main() {
    // init window
    Window window({800, 600}, "grass");
    std::shared_ptr<Input> input = std::make_shared<Input>();
    window.set_input_handler(input);

    // init glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
    }
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0, 0, 800, 600);

    // load shaders
    Shader default_shader;
    default_shader.load_shader_from_path("resources/shaders/default.vert",
                                         "resources/shaders/default.frag");

    // init objects
    glm::mat4 projection = glm::ortho(-8.0f, 8.0f, -8.0f, 8.0f, 0.4f, 100.0f);
    default_shader.set_uniform_matrix4("projection", projection);

    glm::mat4 translation =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
    glm::mat4 rotation =
        glm::rotate(glm::mat4(1.0f), 3.0f, glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 scale = glm::scale(glm::mat4(1.0f), glm::vec3(4.0f));
    glm::mat4 transform = translation * rotation * scale;
    default_shader.set_uniform_matrix4("transform", transform);

    // init render object
    glm::vec3 vertices[6] = {{0.0f, 0.5f, 0.0f},   {1.0f, 0.0f, 0.0f},
                             {-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f},
                             {0.5f, -0.5f, 0.0f},  {0.0f, 0.0f, 1.0f}};

    GLuint vertex_array;
    GLuint vertex_buffer;

    glGenVertexArrays(1, &vertex_array);
    glBindVertexArray(vertex_array);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);

    glVertexAttribPointer(0, 3, GL_FLOAT, 0, 2 * sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, 0, 2 * sizeof(glm::vec3),
                          (void*)sizeof(glm::vec3));
    gl_check_error();
    glEnableVertexAttribArray(1);

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glBindVertexArray(0);

    while (window.is_open()) {
        window.poll_events();
        if (input->is_key_pressed(GLFW_KEY_ESCAPE)) {
            window.close();
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(default_shader.get_id());
        glBindVertexArray(vertex_array);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 3);
        glBindVertexArray(0);
        glUseProgram(0);

        window.display();
    }

    return 0;
}