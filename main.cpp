#include "glad/glad.h"
#include "glm/detail/qualifier.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/trigonometric.hpp"
#include "include/renderer.hpp"
#include "include/window.hpp"
#include <memory>
#include <random>
#include <vector>

int main() {
    // random function
    std::function random_range = [](float low, float high) {
        std::random_device random_device;
        std::mt19937 gen(random_device());
        std::uniform_int_distribution<> distribute(low, high);
        return distribute(gen);
    };

    // init window
    Window window({800, 600}, "grass");
    std::shared_ptr<Input> input = std::make_shared<Input>();
    window.set_input_handler(input);

    // init glad
    Renderer renderer;
    glViewport(0, 0, 800, 600);

    // load shaders
    Shader default_shader;
    default_shader.load_shader_from_path("resources/shaders/default.vert",
                                         "resources/shaders/default.frag");

    // init objects
    std::vector<glm::mat4> grass_transforms;
    for (float x = -4.75f; x < 4.5f; x += 0.48f) {
        for (float z = -4.75f; z < 4.5f; z += 0.48f) {
            float offset_direction = random_range(0.0f, M_PI * 2.0f);
            glm::vec3 offset(cos(offset_direction), 0.0f,
                             sin(offset_direction));
            glm::mat4 translation = glm::translate(
                glm::mat4(1.0f), glm::vec3(x, 0.0f, z) + offset * 0.125f);
            float theta = random_range(0.0f, M_PI);
            glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), theta,
                                             glm::vec3(0.0f, 1.0f, 0.0f));
            grass_transforms.push_back(translation * rotation);
        }
    }

    // init render object
    std::vector<Vertex> grass_vertices = {
        {{0.24f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.25f, 0.0f}},
        {{0.20f, 1.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.5f, 0.0f}},
        {{0.10f, 2.7f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.75f, 0.0f}},
        {{0.0f, 3.2f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.10f, 2.7f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.75f, 0.0f}},
        {{-0.20f, 1.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.5f, 0.0f}},
        {{-0.24f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.25f, 0.0f}},
        {{-0.24f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.25f, 0.0f}},
        {{-0.20f, 1.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.5f, 0.0f}},
        {{-0.10f, 2.7f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.75f, 0.0f}},
        {{0.0f, 3.2f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.10f, 2.7f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.75f, 0.0f}},
        {{0.20f, 1.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.5f, 0.0f}},
        {{0.24f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.25f, 0.0f}},
    };
    std::vector<int> grass_indices = {0,  1, 5, 0,  5, 6,  1,  2,  5,  2,
                                      4,  5, 2, 3,  4, 7,  8,  12, 7,  12,
                                      13, 8, 9, 12, 9, 11, 12, 9,  10, 11};
    Mesh grass_mesh(grass_vertices, grass_indices);

    std::vector<Vertex> ground_vertices = {
        {{5.0f, 0.0f, 5.0f}, {0.0f, 1.0f, 0.0f}, {0.3f, 0.4f, 0.2f}},
        {{5.0f, 0.0f, -5.0f}, {0.0f, 1.0f, 0.0f}, {0.3f, 0.4f, 0.2f}},
        {{-5.0f, 0.0f, -5.0f}, {0.0f, 1.0f, 0.0f}, {0.3f, 0.4f, 0.2f}},
        {{-5.0f, 0.0f, 5.0f}, {0.0f, 1.0f, 0.0f}, {0.3f, 0.4f, 0.2f}},
    };
    std::vector<int> ground_indices = {0, 1, 2, 0, 2, 3};
    Mesh ground_mesh(ground_vertices, ground_indices);

    // camera
    Camera camera(glm::vec3(0.0f, 10.0f, 10.0f), glm::radians(60.0f),
                  8.0f / 6.0f);

    // time
    Timer delta_timer;
    Timer fixed_timer;
    float delta_time = 0.001f;

    while (window.is_open()) {
        window.poll_events();
        if (input->is_key_pressed(GLFW_KEY_ESCAPE)) {
            window.close();
        }

        camera.set_position(glm::vec3(0.0f, 10.0f, 0.0f) +
                            glm::vec3(cos(fixed_timer.get_time() * 0.25f), 0.0f,
                                      sin(fixed_timer.get_time() * 0.25f)) *
                                10.0f);
        camera.look_at(glm::vec3(0.0f, 0.0f, 0.0f));
        renderer.set_camera(camera);

        glClearColor(0.3f, 0.8f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (const glm::mat4 transform : grass_transforms) {
            renderer.draw(grass_mesh, transform, default_shader);
        }
        renderer.draw(ground_mesh, glm::mat4(1.0f), default_shader);

        window.display();

        delta_time = delta_timer.reset();
    }

    return 0;
}