#include "glad/glad.h"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "include/mesh.hpp"
#include "include/shader.hpp"
#include "include/window.hpp"
#include <GL/gl.h>
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
    std::vector<Vertex> vertices = {{{0.0f, 0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}},
                                    {{-0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}},
                                    {{0.5f, -0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}}};

    Mesh mesh(vertices);

    while (window.is_open()) {
        window.poll_events();
        if (input->is_key_pressed(GLFW_KEY_ESCAPE)) {
            window.close();
        }

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(default_shader.get_id());
        glBindVertexArray(mesh.get_vertex_array_id());
        glDrawArrays(GL_TRIANGLE_FAN, 0, 3);
        glBindVertexArray(0);
        glUseProgram(0);

        window.display();
    }

    return 0;
}