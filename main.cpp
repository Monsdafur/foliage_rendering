#include "glad/glad.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/ext/vector_int3.hpp"
#include "glm/trigonometric.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "include/renderer.hpp"
#include "include/window.hpp"
#include "texture.hpp"
#include <cmath>
#include <memory>
#include <random>
#include <vector>

int main() {
    // scene settings
    int field_size = 400;
    int grass_count = field_size * field_size;
    float spacing = 0.4f;
    glm::vec3 start_position = glm::vec3(field_size / -2.0f * spacing, 0.0f,
                                         field_size / -2.0f * spacing);
    float fog_bias = -0.3f;
    glm::vec3 light_direction = glm::vec3(2.0f, -2.6f, 1.8f);
    glm::vec3 fog_color(0.9f, 0.9f, 0.9f);
    glm::vec2 wind_direction = glm::vec2(0.5f, 1.0f);
    float wind_speed = 2.0f;
    bool open_debug_window = true;
    float camera_angle = 270.0f;
    float camera_distance = 15.0f;
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

    // camera
    Camera camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::radians(60.0f),
                  (float)window.get_size().x / (float)window.get_size().y, 0.1f,
                  60.0f);

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
    default_shader.load_shader_from_path(
        "resources/shaders/default_vertex.glsl", GL_VERTEX_SHADER);
    default_shader.load_shader_from_path(
        "resources/shaders/default_fragment.glsl", GL_FRAGMENT_SHADER);

    Shader gpu_instancing_shader;
    gpu_instancing_shader.load_shader_from_path(
        "resources/shaders/gpu_instancing.glsl", GL_VERTEX_SHADER);
    gpu_instancing_shader.load_shader_from_path(
        "resources/shaders/default_fragment.glsl", GL_FRAGMENT_SHADER);

    Shader field_generator_shader;
    field_generator_shader.load_shader_from_path(
        "resources/shaders/field_generator.glsl", GL_COMPUTE_SHADER);

    Shader sine_map_shader;
    sine_map_shader.load_shader_from_path("resources/shaders/sine_map.glsl",
                                          GL_COMPUTE_SHADER);

    Shader displacement;
    displacement.load_shader_from_path("resources/shaders/displacement.glsl",
                                       GL_COMPUTE_SHADER);

    default_shader.set_uniform_vector3("light_direction", light_direction);
    default_shader.set_uniform_vector3("fog_color", fog_color);
    default_shader.set_uniform_float("bias", 0.4f);
    default_shader.set_uniform_float("view_distance",
                                     camera.get_far_clip_plane() - 20.0f);
    default_shader.set_uniform_float("flog_bias", fog_bias);

    gpu_instancing_shader.set_uniform_vector3("light_direction",
                                              light_direction);
    gpu_instancing_shader.set_uniform_vector3("fog_color", fog_color);
    gpu_instancing_shader.set_uniform_float("bias", 0.4f);
    gpu_instancing_shader.set_uniform_float(
        "view_distance", camera.get_far_clip_plane() - 20.0f);
    gpu_instancing_shader.set_uniform_float("flog_bias", fog_bias);
    gpu_instancing_shader.set_uniform_vector2("wind_direction",
                                              glm::normalize(wind_direction));

    field_generator_shader.set_uniform_int("width", field_size);
    field_generator_shader.set_uniform_int("height", field_size);
    field_generator_shader.set_uniform_vector3("start_position",
                                               start_position);
    field_generator_shader.set_uniform_float("spacing", spacing);

    displacement.set_uniform_int("width", field_size);
    displacement.set_uniform_int("height", field_size);
    displacement.set_uniform_int("field_size", field_size * spacing);
    displacement.set_uniform_vector3("start_position", start_position);

    sine_map_shader.set_uniform_vector2("wind_direction",
                                        glm::normalize(wind_direction));

    // textures
    Texture sine_map;
    sine_map.load_texture_from_byte(0, GL_FLOAT, glm::ivec2(512, 512),
                                    GL_RGBA32F, GL_RGBA);
    sine_map.set_filter_mode(GL_LINEAR);

    // init objects
    struct GrassBuffer {
        glm::mat4 transform;
        glm::mat4 sway;
    };
    std::vector<GrassBuffer> grass_transforms(grass_count);

    ShaderBuffer<GrassBuffer> grass_buffer(grass_transforms);
    field_generator_shader.dispatch_buffer_data(
        grass_buffer, glm::ivec3(field_size, field_size, 1));

    std::vector<glm::mat4> cubes(10);
    for (int i = 0; i < cubes.size(); ++i) {
        float w = random_range(1.0f, 2.0f);
        glm::vec3 scale(w, random_range(3.0f, 5.0f), w);
        glm::vec3 position(
            random_range(-field_size * spacing, field_size * spacing) * 0.5f,
            scale.y,
            random_range(-field_size * spacing, field_size * spacing) * 0.5f);
        glm::vec3 rotation_axis(0.0f, 1.0f, 0.0f);
        float angle = random_range(0.0, M_PI * 2.0f);
        glm::mat4 m_i(1.0f);
        cubes[i] = glm::translate(m_i, position) *
                   glm::rotate(m_i, angle, rotation_axis) *
                   glm::scale(m_i, scale);
    }

    // init render object
    std::vector<Vertex> cube_vertices = {
        // front face
        {{-1.0f, -1.0f, 1.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, 1.0f},
         {1.0f, 1.0f, 0.7f}},
        {{1.0f, -1.0f, 1.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, 1.0f},
         {1.0f, 1.0f, 0.7f}},
        {{1.0f, 1.0f, 1.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, 1.0f},
         {1.0f, 1.0f, 0.7f}},
        {{-1.0f, 1.0f, 1.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, 1.0f},
         {1.0f, 1.0f, 0.7f}},
        // back face
        {{1.0f, -1.0f, -1.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, -1.0f},
         {1.0f, 1.0f, 0.7f}},
        {{-1.0f, -1.0f, -1.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, -1.0f},
         {1.0f, 1.0f, 0.7f}},
        {{-1.0f, 1.0f, -1.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, -1.0f},
         {1.0f, 1.0f, 0.7f}},
        {{1.0f, 1.0f, -1.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, -1.0f},
         {1.0f, 1.0f, 0.7f}},
        // left face
        {{-1.0f, -1.0f, -1.0f},
         {0.0f, 0.0f},
         {-1.0f, 0.0f, 0.0f},
         {1.0f, 1.0f, 0.7f}},
        {{-1.0f, -1.0f, 1.0f},
         {0.0f, 0.0f},
         {-1.0f, 0.0f, 0.0f},
         {1.0f, 1.0f, 0.7f}},
        {{-1.0f, 1.0f, 1.0f},
         {0.0f, 0.0f},
         {-1.0f, 0.0f, 0.0f},
         {1.0f, 1.0f, 0.7f}},
        {{-1.0f, 1.0f, -1.0f},
         {0.0f, 0.0f},
         {-1.0f, 0.0f, 0.0f},
         {1.0f, 1.0f, 0.7f}},
        // right face
        {{1.0f, -1.0f, 1.0f},
         {0.0f, 0.0f},
         {1.0f, 0.0f, 0.0f},
         {1.0f, 1.0f, 0.7f}},
        {{1.0f, -1.0f, -1.0f},
         {0.0f, 0.0f},
         {1.0f, 0.0f, 0.0f},
         {1.0f, 1.0f, 0.7f}},
        {{1.0f, 1.0f, -1.0f},
         {0.0f, 0.0f},
         {1.0f, 0.0f, 0.0f},
         {1.0f, 1.0f, 0.7f}},
        {{1.0f, 1.0f, 1.0f},
         {0.0f, 0.0f},
         {1.0f, 0.0f, 0.0f},
         {1.0f, 1.0f, 0.7f}},
        // top face
        {{-1.0f, 1.0f, 1.0f},
         {0.0f, 0.0f},
         {0.0f, 1.0f, 0.0f},
         {1.0f, 1.0f, 0.7f}},
        {{1.0f, 1.0f, 1.0f},
         {0.0f, 0.0f},
         {0.0f, 1.0f, 0.0f},
         {1.0f, 1.0f, 0.7f}},
        {{1.0f, 1.0f, -1.0f},
         {0.0f, 0.0f},
         {0.0f, 1.0f, 0.0f},
         {1.0f, 1.0f, 0.7f}},
        {{-1.0f, 1.0f, -1.0f},
         {0.0f, 0.0f},
         {0.0f, 1.0f, 0.0f},
         {1.0f, 1.0f, 0.7f}},
        // bottom face
        {{-1.0f, -1.0f, -1.0f},
         {0.0f, 0.0f},
         {0.0f, -1.0f, 0.0f},
         {1.0f, 0.7f, 0.4f}},
        {{1.0f, -1.0f, -1.0f},
         {0.0f, 0.0f},
         {0.0f, -1.0f, 0.0f},
         {1.0f, 0.7f, 0.4f}},
        {{1.0f, -1.0f, 1.0f},
         {0.0f, 0.0f},
         {0.0f, -1.0f, 0.0f},
         {1.0f, 0.7f, 0.4f}},
        {{-1.0f, -1.0f, 1.0f},
         {0.0f, 0.0f},
         {0.0f, -1.0f, 0.0f},
         {1.0f, 0.7f, 0.4f}},
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
        {{0.24f, 0.0f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, 1.0f},
         {0.0f, 0.75f, 0.0f}},
        {{0.20f, 0.5f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, 1.0f},
         {0.0f, 0.75f, 0.0f}},
        {{0.10f, 0.8f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, 1.0f},
         {0.0f, 0.75f, 0.0f}},
        {{0.0f, 1.0f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, 1.0f},
         {0.0f, 0.75f, 0.0f}},
        {{-0.10f, 0.8f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, 1.0f},
         {0.0f, 0.75f, 0.0f}},
        {{-0.20f, 0.5f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, 1.0f},
         {0.0f, 0.75f, 0.0f}},
        {{-0.24f, 0.0f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, 1.0f},
         {0.0f, 0.75f, 0.0f}},
        // back
        {{-0.24f, 0.0f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, -1.0f},
         {0.0f, 0.75f, 0.0f}},
        {{-0.20f, 0.5f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, -1.0f},
         {0.0f, 0.75f, 0.0f}},
        {{-0.10f, 0.8f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, -1.0f},
         {0.0f, 0.75f, 0.0f}},
        {{0.0f, 1.0f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, -1.0f},
         {0.0f, 0.75f, 0.0f}},
        {{0.10f, 0.8f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, -1.0f},
         {0.0f, 0.675, 0.0f}},
        {{0.20f, 0.5f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, -1.0f},
         {0.0f, 0.675, 0.0f}},
        {{0.24f, 0.0f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, -1.0f},
         {0.0f, 0.675, 0.0f}},
    };
    std::vector<int> grass_indices = {
        // front
        0, 1, 5, 0, 5, 6, 1, 2, 5, 2, 4, 5, 2, 3, 4,
        // back
        7, 8, 12, 7, 12, 13, 8, 9, 12, 9, 11, 12, 9, 10, 11};

    std::vector<Vertex> ground_vertices = {
        {{1.0f, 0.0f, 1.0f},
         {0.0f, 0.0f},
         {0.0f, 1.0f, 0.0f},
         {0.1f, 0.06f, 0.0f}},
        {{1.0f, 0.0f, -1.0f},
         {0.0f, 0.0f},
         {0.0f, 1.0f, 0.0f},
         {0.1f, 0.06f, 0.0f}},
        {{-1.0f, 0.0f, -1.0f},
         {0.0f, 0.0f},
         {0.0f, 1.0f, 0.0f},
         {0.1f, 0.06f, 0.0f}},
        {{-1.0f, 0.0f, 1.0f},
         {0.0f, 0.0f},
         {0.0f, 1.0f, 0.0f},
         {0.1f, 0.06f, 0.0f}},
    };
    std::vector<int> ground_indices = {0, 1, 2, 0, 2, 3};

    std::vector<Vertex> image_vertices = {
        {{2.0f, 2.0f, 0.0f},
         {1.0f, 1.0f},
         {0.0f, 0.0f, 1.0f},
         {1.0f, 1.0f, 1.0f}},
        {{-2.0f, 2.0f, 0.0f},
         {0.0f, 1.0f},
         {0.0f, 0.0f, 1.0f},
         {1.0f, 1.0f, 1.0f}},
        {{-2.0f, -2.0f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, 1.0f},
         {1.0f, 1.0f, 1.0f}},
        {{2.0f, -2.0f, 0.0f},
         {1.0f, 0.0f},
         {0.0f, 0.0f, 1.0f},
         {1.0f, 1.0f, 1.0f}},
        {{2.0f, -2.0f, 0.0f},
         {1.0f, 0.0f},
         {0.0f, 0.0f, -1.0f},
         {1.0f, 1.0f, 1.0f}},
        {{-2.0f, -2.0f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, -1.0f},
         {1.0f, 1.0f, 1.0f}},
        {{-2.0f, 2.0f, 0.0f},
         {0.0f, 1.0f},
         {0.0f, 0.0f, -1.0f},
         {1.0f, 1.0f, 1.0f}},
        {{2.0f, 2.0f, 0.0f},
         {1.0f, 1.0f},
         {0.0f, 0.0f, -1.0f},
         {1.0f, 1.0f, 1.0f}},
    };
    std::vector<int> image_indices = {0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7};

    Mesh cube_mesh(cube_vertices, cube_indices);
    Mesh grass_mesh(grass_vertices, grass_indices);
    Mesh ground_mesh(ground_vertices, ground_indices);
    Mesh image_mesh(image_vertices, image_indices);

    image_mesh.set_texture(std::make_shared<const Texture>(sine_map));

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
            ImGui::SetNextWindowSize(ImVec2(320.0f, 640.0f));
            ImGui::Begin("debug", &open_debug_window,
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            if (fixed_timer.get_time() > next_fps_update) {
                fps = (int)(1.0f / delta_time);
                push_array(fps_over_time, 64, fps);
                next_fps_update = fixed_timer.get_time() + fps_update_interval;
            }
            ImGui::Text("grass blade count: %zu", grass_transforms.size());
            ImGui::Text("FPS: %d", fps);
            ImGui::PlotLines("FPS over time", fps_over_time, 64);
            ImGui::SliderFloat("angle", &camera_angle, 0.0f, 360.0f);
            ImGui::SliderFloat("height", &camera_height, 2.0f, 48.0f);
            ImGui::SliderFloat("distance", &camera_distance, 5.0f,
                               camera.get_far_clip_plane());
            ImGui::SliderFloat("wind speed", &wind_speed, -20.0f, 20.0f);
            ImGui::SliderFloat("fog bias", &fog_bias, -1.0f, 1.0f);

            ImGui::Text("displacement map");
            ImTextureID imgui_texture = sine_map.get_id();
            ImGui::Image(imgui_texture, ImVec2(256.0f, 256.0f));

            ImGui::End();
        }

        // update logic
        camera.set_position(glm::vec3(0.0f, camera_height, 0.0f) +
                            glm::vec3(cos(glm::radians(camera_angle)), 0.0f,
                                      sin(glm::radians(camera_angle))) *
                                camera_distance);
        camera.look_at(glm::vec3(0.0f, 0.0f, 0.0f));
        renderer.set_camera(camera);

        default_shader.set_uniform_vector3("camera_position",
                                           camera.get_position());
        gpu_instancing_shader.set_uniform_vector3("camera_position",
                                                  camera.get_position());
        default_shader.set_uniform_float("fog_bias", fog_bias);
        gpu_instancing_shader.set_uniform_float("fog_bias", fog_bias);

        sine_map_shader.set_uniform_float("shift",
                                          fixed_timer.get_time() * wind_speed);
        sine_map_shader.dispatch_texture(sine_map,
                                         glm::ivec3(sine_map.get_size(), 1));
        displacement.set_uniform_texture("sine_map_shader_map", sine_map, 0);
        displacement.dispatch_buffer_data(
            grass_buffer, glm::ivec3(field_size, field_size, 1));

        // render
        glClearColor(fog_color.r, fog_color.g, fog_color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderer.draw_instances(grass_mesh, grass_buffer, gpu_instancing_shader,
                                grass_transforms.size());

        renderer.draw(ground_mesh,
                      glm::scale(glm::mat4(1.0f), glm::vec3(200.0f)),
                      default_shader);
        for (glm::mat4 cube : cubes) {
            renderer.draw(cube_mesh, cube, default_shader);
        }

        // renderer.draw(
        //     image_mesh,
        //     glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 6.0f, 0.0f)),
        //     default_shader);

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