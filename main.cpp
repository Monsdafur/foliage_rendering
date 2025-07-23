#include "glad/glad.h"
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
    // scene settings
    float field_size = 50.0f;
    float spacing = 0.4f;
    float fog_bias = 20.0f;
    glm::vec3 light_direction = glm::vec3(1.0f, -2.3f, 1.4f);
    glm::vec3 fog_color(0.9f, 0.9f, 0.9f);

    // random function
    std::function random_range = [](float low, float high) {
        std::random_device random_device;
        std::mt19937 gen(random_device());
        std::uniform_real_distribution<> distribute(low, high);
        return distribute(gen);
    };

    // init window
    Window window({1920, 1080}, "grass");
    std::shared_ptr<Input> input = std::make_shared<Input>();
    window.set_input_handler(input);

    // init renderer
    Renderer renderer;
    glViewport(0, 0, 1920, 1080);

    // load shaders
    Shader default_shader;
    default_shader.load_shader_from_path("resources/shaders/default.vert",
                                         "resources/shaders/default.frag");

    Shader gpu_instancing_shader;
    gpu_instancing_shader.load_shader_from_path(
        "resources/shaders/gpu_instancing.vert",
        "resources/shaders/gpu_instancing.frag");

    default_shader.set_uniform_vector3("light_direction", light_direction);
    default_shader.set_uniform_vector3("fog_color", fog_color);
    default_shader.set_uniform_float("bias", 0.4f);
    default_shader.set_uniform_float("near", 0.1f);
    default_shader.set_uniform_float("far", 40.0f);
    default_shader.set_uniform_float("fog_bias", fog_bias);

    gpu_instancing_shader.set_uniform_vector3("light_direction",
                                              light_direction);
    gpu_instancing_shader.set_uniform_vector3("fog_color", fog_color);
    gpu_instancing_shader.set_uniform_float("bias", 0.8f);
    gpu_instancing_shader.set_uniform_float("near", 0.1f);
    gpu_instancing_shader.set_uniform_float("far", 40.0f);
    gpu_instancing_shader.set_uniform_float("fog_bias", fog_bias);

    // init objects
    std::vector<BufferData> grass_transforms;
    for (float x = -field_size; x < field_size; x += spacing) {
        for (float z = -field_size; z < field_size; z += spacing) {
            float offset_direction = random_range(0.0f, M_PI * 2.0f);
            glm::vec3 offset(cos(offset_direction), 0.0f,
                             sin(offset_direction));
            glm::mat4 translation = glm::translate(
                glm::mat4(1.0f), glm::vec3(x, 0.0f, z) + offset * 0.2f);
            glm::mat4 rotation_y =
                glm::rotate(glm::mat4(1.0f), (float)random_range(0.0f, M_PI),
                            glm::vec3(0.0f, 1.0f, 0.0f));
            glm::mat4 rotation_x =
                glm::rotate(glm::mat4(1.0f), (float)random_range(-0.1f, 0.1f),
                            glm::vec3(1.0f, 0.0f, 0.0f));
            glm::mat4 rotation = rotation_x * rotation_y;
            glm::mat4 scale =
                glm::scale(glm::mat4(1.0f),
                           glm::vec3(1.0f, random_range(0.9f, 1.0f), 1.0f));
            grass_transforms.push_back({translation * rotation * scale});
        }
    }
    ShaderBuffer grass_buffer(grass_transforms);

    // init render object
    std::vector<Vertex> cube_vertices = {
        // front face
        {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.7f}},
        {{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.7f}},
        {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.7f}},
        {{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.7f}},

        // back face
        {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 0.7f}},
        {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 0.7f}},
        {{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 0.7f}},
        {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 0.7f}},

        // left face
        {{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},
        {{-1.0f, -1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},
        {{-1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},
        {{-1.0f, 1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},

        // right face
        {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},
        {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},
        {{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},
        {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},

        // top face
        {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},
        {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},
        {{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},
        {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},

        // bottom face
        {{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},
        {{1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},
        {{1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},
        {{-1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},
    };

    std::vector<int> cube_indices = {// front
                                     0, 1, 2, 2, 3, 0,
                                     // back
                                     4, 5, 6, 6, 7, 4,
                                     // left
                                     8, 9, 10, 10, 11, 8,
                                     // right
                                     12, 13, 14, 14, 15, 12,
                                     // top
                                     16, 17, 18, 18, 19, 16,
                                     // bottom
                                     20, 21, 22, 22, 23, 20};

    std::vector<Vertex> grass_vertices = {
        // front
        {{0.24f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.25f, 0.0f}},
        {{0.20f, 1.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.5f, 0.0f}},
        {{0.10f, 2.7f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.75f, 0.0f}},
        {{0.0f, 3.2f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.10f, 2.7f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.75f, 0.0f}},
        {{-0.20f, 1.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.5f, 0.0f}},
        {{-0.24f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.25f, 0.0f}},
        // back
        {{-0.24f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.25f, 0.0f}},
        {{-0.20f, 1.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.5f, 0.0f}},
        {{-0.10f, 2.7f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.75f, 0.0f}},
        {{0.0f, 3.2f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.10f, 2.7f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.75f, 0.0f}},
        {{0.20f, 1.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.5f, 0.0f}},
        {{0.24f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.25f, 0.0f}},
    };
    std::vector<int> grass_indices = {
        // front
        0, 1, 5, 0, 5, 6, 1, 2, 5, 2, 4, 5, 2, 3, 4,
        // back
        7, 8, 12, 7, 12, 13, 8, 9, 12, 9, 11, 12, 9, 10, 11};

    std::vector<Vertex> ground_vertices = {
        {{1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.3f, 0.4f, 0.2f}},
        {{1.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.3f, 0.4f, 0.2f}},
        {{-1.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.3f, 0.4f, 0.2f}},
        {{-1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.3f, 0.4f, 0.2f}},
    };
    std::vector<int> ground_indices = {0, 1, 2, 0, 2, 3};

    Mesh cube_mesh(cube_vertices, cube_indices);
    Mesh grass_mesh(grass_vertices, grass_indices);
    Mesh ground_mesh(ground_vertices, ground_indices);

    // camera
    Camera camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::radians(60.0f),
                  (float)window.get_size().x / (float)window.get_size().y, 0.1f,
                  40.0f);

    // time
    Timer delta_timer;
    Timer fixed_timer;
    float delta_time = 0.001f;

    while (window.is_open()) {
        window.poll_events();
        if (input->is_key_pressed(GLFW_KEY_ESCAPE)) {
            window.close();
        }

        camera.set_position(glm::vec3(0.0f, 7.0f, 0.0f) +
                            glm::vec3(cos(fixed_timer.get_time() * 0.25f), 0.0f,
                                      sin(fixed_timer.get_time() * 0.25f)) *
                                20.0f);
        camera.look_at(glm::vec3(0.0f, 0.0f, 0.0f));
        renderer.set_camera(camera);
        default_shader.set_uniform_vector3("camera_position",
                                           camera.get_position());

        glClearColor(fog_color.r, fog_color.g, fog_color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderer.draw_instances(grass_mesh, grass_buffer, gpu_instancing_shader,
                                grass_transforms.size());

        renderer.draw(ground_mesh,
                      glm::scale(glm::mat4(1.0f), glm::vec3(field_size)),
                      default_shader);

        renderer.draw(
            cube_mesh,
            glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.5f, 0.0f)) *
                glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 2.5f, 1.0f)),
            default_shader);

        window.display();

        delta_time = delta_timer.reset();
    }

    return 0;
}