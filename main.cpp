#include "PerlinNoise.hpp"
#include "glad/glad.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "glm/ext/vector_int3.hpp"
#include "glm/trigonometric.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "include/renderer.hpp"
#include "include/window.hpp"
#include "texture.hpp"
#include <cmath>
#include <cstdint>
#include <memory>
#include <random>
#include <vector>

int main() {
    // scene settings
    glm::vec3 lower_bound(-100.0f, 0.0f, -100.0f);
    glm::vec3 upper_bound(100.0f, 0.0f, 100.0f);
    assert(upper_bound.x > lower_bound.x);
    assert(upper_bound.z > lower_bound.z);
    int grass_per_unit = 2;
    int grass_count = (upper_bound.x - lower_bound.x) *
                      (upper_bound.z - lower_bound.z) * grass_per_unit *
                      grass_per_unit;
    glm::ivec3 size = glm::ivec3(upper_bound - lower_bound) * grass_per_unit;
    float fog_bias = -0.3f;
    glm::vec3 light_direction = glm::vec3(2.0f, -2.6f, 1.8f);
    glm::vec3 fog_color(0.9f, 0.9f, 0.9f);
    float wind_angle = 140.0f;
    glm::vec2 wind_direction =
        glm::vec2(cos(glm::radians(wind_angle)), sin(glm::radians(wind_angle)));
    float wind_speed = 2.0f;
    bool open_debug_window = true;
    float camera_angle = 270.0f;
    float camera_distance = 15.0f;
    float camera_height = 20.0f;
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
    Window window({1920, 1080}, "grass");
    std::shared_ptr<Input> input = std::make_shared<Input>();
    window.set_input_handler(input);

    // init renderer
    Renderer renderer;
    glViewport(0, 0, window.get_size().x, window.get_size().y);

    // camera
    Camera camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::radians(60.0f),
                  (float)window.get_size().x / (float)window.get_size().y, 0.1f,
                  100.0f);

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

    Shader post_processing;
    post_processing.load_shader_from_path(
        "resources/shaders/screen_vertex.glsl", GL_VERTEX_SHADER);
    post_processing.load_shader_from_path(
        "resources/shaders/post_processing.glsl", GL_FRAGMENT_SHADER);

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

    field_generator_shader.set_uniform_int("width", size.x);
    field_generator_shader.set_uniform_int("height", size.z);
    field_generator_shader.set_uniform_vector3("lower_bound", lower_bound);
    field_generator_shader.set_uniform_vector3("upper_bound", upper_bound);
    field_generator_shader.set_uniform_float("spacing", 1.0f / grass_per_unit);

    displacement.set_uniform_int("width", size.x);
    displacement.set_uniform_int("height", size.z);

    // textures
    Texture sine_map;
    sine_map.load_texture_from_byte(0, GL_FLOAT, glm::ivec2(size.x, size.z),
                                    GL_RGBA32F, GL_RGBA);
    sine_map.set_filter_mode(GL_LINEAR);

    RenderTexture screen_texture(window.get_size(), GL_RGB);
    screen_texture.set_filter_mode(GL_NEAREST);

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
         {0.5f, 0.75f, 0.0f}},
        {{0.0f, 1.0f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, 1.0f},
         {0.9f, 0.75f, 0.0f}},
        {{-0.10f, 0.8f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, 1.0f},
         {0.5f, 0.75f, 0.0f}},
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
         {0.5f, 0.75f, 0.0f}},
        {{0.0f, 1.0f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, -1.0f},
         {0.9f, 0.75f, 0.0f}},
        {{0.10f, 0.8f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, -1.0f},
         {0.5f, 0.75f, 0.0f}},
        {{0.20f, 0.5f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, -1.0f},
         {0.0f, 0.75f, 0.0f}},
        {{0.24f, 0.0f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, -1.0f},
         {0.0f, 0.75f, 0.0f}},
    };
    std::vector<int> grass_indices = {
        // front
        0, 1, 5, 0, 5, 6, 1, 2, 5, 2, 4, 5, 2, 3, 4,
        // back
        7, 8, 12, 7, 12, 13, 8, 9, 12, 9, 11, 12, 9, 10, 11};

    std::vector<Vertex> ground_vertices;
    std::vector<int> ground_indices;

    glm::ivec2 sub_divide(64, 64);
    glm::vec3 spacing((upper_bound.x - lower_bound.x) / (float)sub_divide.x,
                      0.0f,
                      ((upper_bound.z - lower_bound.z) / (float)sub_divide.y));
    float scale = 40.0f;
    int indices = 0;
    uint8_t bytes[sub_divide.x * sub_divide.y * 3];
    int bi = 0;

    const siv::PerlinNoise::seed_type seed = 123456u;

    const siv::PerlinNoise perlin{seed};

    for (int x = 0; x < sub_divide.x; ++x) {
        for (int y = 0; y < sub_divide.y; ++y) {
            int x1 = x + 1;
            int y1 = y + 1;
            // float height = sin(x * 0.25f);
            float h0 =
                perlin.octave2D_01((x1 * 0.02f), (y1 * 0.02f), 4) * scale;
            float h1 = perlin.octave2D_01((x1 * 0.02f), (y * 0.02f), 4) * scale;
            float h2 = perlin.octave2D_01((x * 0.02f), (y * 0.02f), 4) * scale;
            float h3 = perlin.octave2D_01((x * 0.02f), (y1 * 0.02f), 4) * scale;
            glm::vec3 v0(lower_bound.x + x1 * spacing.x, h0,
                         lower_bound.z + y1 * spacing.z);
            glm::vec3 v1(lower_bound.x + x1 * spacing.x, h1,
                         lower_bound.z + y * spacing.z);
            glm::vec3 v2(lower_bound.x + x * spacing.x, h2,
                         lower_bound.z + y * spacing.z);
            glm::vec3 v3(lower_bound.x + x * spacing.x, h3,
                         lower_bound.z + y1 * spacing.z);
            glm::vec3 normal = glm::normalize(glm::cross(v2 - v1, v0 - v1));
            glm::vec3 color(0.1f, 0.06f, 0.0f);
            ground_vertices.push_back({v0, {0.0f, 0.0f}, normal, color});
            ground_vertices.push_back({v1, {0.0f, 0.0f}, normal, color});
            ground_vertices.push_back({v2, {0.0f, 0.0f}, normal, color});
            ground_vertices.push_back({v3, {0.0f, 0.0f}, normal, color});
            ground_indices.push_back(indices);
            ground_indices.push_back(indices + 1);
            ground_indices.push_back(indices + 2);
            ground_indices.push_back(indices);
            ground_indices.push_back(indices + 2);
            ground_indices.push_back(indices + 3);
            indices += 4;

            int color_index = (x + sub_divide.x * y) * 3;
            bytes[color_index] = (h0 / scale) * 255.0f;
            bytes[color_index + 1] = (h0 / scale) * 255.0f;
            bytes[color_index + 2] = (h0 / scale) * 255.0f;
        }
    }
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    Texture height_map;
    height_map.load_texture_from_byte(bytes, GL_UNSIGNED_BYTE, sub_divide,
                                      GL_RGB, GL_RGB);
    height_map.set_filter_mode(GL_LINEAR);

    struct GrassBuffer {
        glm::mat4 transform;
        glm::mat4 sway;
    };
    std::vector<GrassBuffer> grass_transforms(grass_count);

    field_generator_shader.set_uniform_texture("height_map", height_map, 0);
    field_generator_shader.set_uniform_float("terrain_scale", scale);
    ShaderBuffer<GrassBuffer> grass_buffer(grass_transforms);
    field_generator_shader.dispatch_buffer_data(grass_buffer,
                                                glm::ivec3(size.x, size.z, 1));

    std::vector<Vertex> screen_vertices = {{{1.0f, 1.0f, 0.0f},
                                            {1.0f, 1.0f},
                                            {0.0f, 0.0f, 1.0f},
                                            {1.0f, 1.0f, 1.0f}},
                                           {{-1.0f, 1.0f, 0.0f},
                                            {0.0f, 1.0f},
                                            {0.0f, 0.0f, 1.0f},
                                            {1.0f, 1.0f, 1.0f}},
                                           {{-1.0f, -1.0f, 0.0f},
                                            {0.0f, 0.0f},
                                            {0.0f, 0.0f, 1.0f},
                                            {1.0f, 1.0f, 1.0f}},
                                           {{1.0f, -1.0f, 0.0f},
                                            {1.0f, 0.0f},
                                            {0.0f, 0.0f, 1.0f},
                                            {1.0f, 1.0f, 1.0f}}};
    std::vector<int> screen_indices = {0, 1, 2, 0, 2, 3};

    Mesh cube_mesh(cube_vertices, cube_indices);
    Mesh grass_mesh(grass_vertices, grass_indices);
    Mesh ground_mesh(ground_vertices, ground_indices);
    Mesh screen_mesh(screen_vertices, screen_indices);

    screen_mesh.set_texture(std::make_shared<const Texture>(screen_texture));

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
            ImGui::SliderFloat("wind speed", &wind_speed, 0.0f, 20.0f);
            ImGui::SliderFloat("wind direction", &wind_angle, 0.0f, 360.0f);
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
        displacement.dispatch_buffer_data(grass_buffer,
                                          glm::ivec3(size.x, size.z, 1));

        wind_direction = glm::vec2(cos(glm::radians(wind_angle)),
                                   sin(glm::radians(wind_angle)));
        sine_map_shader.set_uniform_vector2("wind_direction", wind_direction);
        gpu_instancing_shader.set_uniform_vector2("wind_direction",
                                                  wind_direction);

        // render to framebuffer
        screen_texture.begin_draw();
        glClearColor(fog_color.r, fog_color.g, fog_color.b, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        renderer.draw_instances(grass_mesh, grass_buffer, gpu_instancing_shader,
                                grass_transforms.size());

        renderer.draw(ground_mesh, glm::mat4(1.0f), default_shader);
        screen_texture.end_draw();

        // draw framebuffer
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.draw(screen_mesh, glm::mat4(1.0f), post_processing);

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