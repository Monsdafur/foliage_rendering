#include "glad/glad.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_float3.hpp"
#include "glm/trigonometric.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "include/renderer.hpp"
#include "include/window.hpp"
#include <memory>
#include <random>
#include <vector>

int main() {
    // scene settings
    float field_size = 55.0f;
    float spacing = 0.4f;
    float fog_bias = 20.0f;
    glm::vec3 light_direction = glm::vec3(2.0f, -2.6f, 1.8f);
    glm::vec3 fog_color(0.9f, 0.9f, 1.0f);
    bool open_debug_window = true;
    float camera_angle = 0.0f;
    float camera_distance = 5.0f;
    float camera_height = 6.0f;
    int fps = 0;
    float fps_update_interval = 0.5f;
    float next_fps_update = fps_update_interval;
    float fps_over_time[64];
    for (int i = 0; i < 64; ++i) {
        fps_over_time[i] = 60.0f;
    }

    auto push_array = [](float* array, int count, float value) {
        for (int i = count - 1; i > 0; --i) {
            array[i] = array[i - 1];
        }
        array[0] = value;
    };

    // random function
    std::function random_range = [](float low, float high) {
        std::random_device random_device;
        std::mt19937 gen(random_device());
        std::uniform_real_distribution<> distribute(low, high);
        return distribute(gen);
    };

    // init window
    Window window({1600, 800}, "grass");
    std::shared_ptr<Input> input = std::make_shared<Input>();
    window.set_input_handler(input);

    // init renderer
    Renderer renderer;
    glViewport(0, 0, window.get_size().x, window.get_size().y);

    // imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui_ImplGlfw_InitForOpenGL(window.get_handler(), true);
    ImGui_ImplOpenGL3_Init("#version 430");

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
    default_shader.set_uniform_float("bias", 0.5f);
    default_shader.set_uniform_float("near", 0.1f);
    default_shader.set_uniform_float("far", 50.0f);
    default_shader.set_uniform_float("fog_bias", fog_bias);

    gpu_instancing_shader.set_uniform_vector3("light_direction",
                                              light_direction);
    gpu_instancing_shader.set_uniform_vector3("fog_color", fog_color);
    gpu_instancing_shader.set_uniform_float("bias", 0.7f);
    gpu_instancing_shader.set_uniform_float("near", 0.1f);
    gpu_instancing_shader.set_uniform_float("far", 50.0f);
    gpu_instancing_shader.set_uniform_float("fog_bias", fog_bias);

    // init objects
    struct GrassBuffer {
        glm::mat4 transform;
        glm::mat4 sway;
    };
    std::vector<GrassBuffer> grass_transforms;
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
                           glm::vec3(1.0f, random_range(1.5f, 4.0f), 1.0f));
            grass_transforms.push_back(
                {translation * rotation * scale,
                 glm::mat4(random_range(-0.05f, 0.16f))});
        }
    }
    ShaderBuffer<GrassBuffer> grass_buffer(grass_transforms);

    // init render object
    std::vector<Vertex> cube_vertices = {
        // front face
        {{-1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.25f, 0.25f, 0.135f}},
        {{1.0f, -1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {0.25f, 0.25f, 0.135f}},
        {{1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.7f}},
        {{-1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 0.7f}},
        // back face
        {{1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.25f, 0.25f, 0.135f}},
        {{-1.0f, -1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {0.25f, 0.25f, 0.135f}},
        {{-1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 0.7f}},
        {{1.0f, 1.0f, -1.0f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f, 0.7f}},
        // left face
        {{-1.0f, -1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {0.25f, 0.25f, 0.135f}},
        {{-1.0f, -1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {0.25f, 0.25f, 0.135f}},
        {{-1.0f, 1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},
        {{-1.0f, 1.0f, -1.0f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},
        // right face
        {{1.0f, -1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {0.25f, 0.25f, 0.135f}},
        {{1.0f, -1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {0.25f, 0.25f, 0.135f}},
        {{1.0f, 1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},
        {{1.0f, 1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},
        // top face
        {{-1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},
        {{1.0f, 1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},
        {{1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},
        {{-1.0f, 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f, 0.7f}},
        // bottom face
        {{-1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.7f, 0.4f}},
        {{1.0f, -1.0f, -1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.7f, 0.4f}},
        {{1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.7f, 0.4f}},
        {{-1.0f, -1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.7f, 0.4f}},
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
        {{0.24f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.20f, 0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.10f, 0.8f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.10f, 0.8f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.20f, 0.5f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.24f, 0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}},
        // back
        {{-0.24f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.20f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{-0.10f, 0.8f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.10f, 0.8f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.20f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
        {{0.24f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}},
    };
    std::vector<int> grass_indices = {
        // front
        0, 1, 5, 0, 5, 6, 1, 2, 5, 2, 4, 5, 2, 3, 4,
        // back
        7, 8, 12, 7, 12, 13, 8, 9, 12, 9, 11, 12, 9, 10, 11};

    std::vector<Vertex> ground_vertices = {
        {{1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.1f, 0.2f, 0.0f}},
        {{1.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.2f, 0.2f, 0.0f}},
        {{-1.0f, 0.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.2f, 0.2f, 0.0f}},
        {{-1.0f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}, {0.1f, 0.2f, 0.0f}},
    };
    std::vector<int> ground_indices = {0, 1, 2, 0, 2, 3};

    Mesh cube_mesh(cube_vertices, cube_indices);
    Mesh grass_mesh(grass_vertices, grass_indices);
    Mesh ground_mesh(ground_vertices, ground_indices);

    // camera
    Camera camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::radians(60.0f),
                  (float)window.get_size().x / (float)window.get_size().y, 0.1f,
                  50.0f);

    // time
    Timer delta_timer;
    Timer fixed_timer;
    float delta_time = 0.016f;

    while (window.is_open()) {
        window.poll_events();
        if (input->is_key_pressed(GLFW_KEY_ESCAPE)) {
            window.close();
        }
        // imgui
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        if (open_debug_window) {
            ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
            ImGui::SetNextWindowSize(ImVec2(320.0f, 320.0f));
            ImGui::Begin("debug", &open_debug_window,
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            if (fixed_timer.get_time() > next_fps_update) {
                fps = (int)(1.0f / delta_time);
                push_array(fps_over_time, 64, fps);
                next_fps_update = fixed_timer.get_time() + fps_update_interval;
            }
            ImGui::Text("FPS: %d", fps);
            ImGui::PlotLines("FPS over time", fps_over_time, 64);
            ImGui::SliderFloat("angle", &camera_angle, 0.0f, 360.0f);
            ImGui::SliderFloat("height", &camera_height, 2.0f, 12.0f);
            ImGui::SliderFloat("distance", &camera_distance, 5.0f, 50.0f);
            ImGui::End();
        }

        // update logic
        camera.set_position(glm::vec3(0.0f, camera_height, 0.0f) +
                            glm::vec3(cos(glm::radians(camera_angle)), 0.0f,
                                      sin(glm::radians(camera_angle))) *
                                camera_distance);
        camera.look_at(glm::vec3(0.0f, 0.0f, 0.0f));
        renderer.set_camera(camera);
        gpu_instancing_shader.set_uniform_float(
            "offset", sin(fixed_timer.get_time() * 4.0f) +
                          sin(fixed_timer.get_time() * 2.5f));

        // render
        glClearColor(fog_color.r, fog_color.g, fog_color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderer.draw_instances(grass_mesh, grass_buffer, gpu_instancing_shader,
                                grass_transforms.size());

        renderer.draw(
            ground_mesh,
            glm::scale(glm::mat4(1.0f), glm::vec3(field_size * 20.0f)),
            default_shader);

        renderer.draw(
            cube_mesh,
            glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 2.5f, 0.0f)) *
                glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 2.5f, 1.0f)),
            default_shader);

        // render imgui
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        window.display();

        delta_time = delta_timer.reset();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    return 0;
}