#include "glad/glad.h"
#include "glm/ext/vector_float3.hpp"
#include "glm/ext/vector_float4.hpp"
#include "glm/ext/vector_int3.hpp"
#include "glm/matrix.hpp"
#include "glm/trigonometric.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "include/chunk.hpp"
#include "include/mesh.hpp"
#include "renderer.hpp"
#include "window.hpp"
#include <GL/gl.h>
#include <memory>

int main() {
    // init window
    Window window(glm::ivec2(1920, 1080), "chunking test");
    Renderer renderer;
    Camera camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::radians(60.0f),
                  (float)window.get_size().x / (float)window.get_size().y, 0.1f,
                  100.0f);

    Camera camera2(glm::vec3(0.0f, 40.0f, 0.0f),
                   glm::vec2(window.get_size()) * 0.1f, 0.1f, 100.0f);

    renderer.set_camera(camera);
    glViewport(0, 0, window.get_size().x, window.get_size().y);

    std::shared_ptr<Input> input = std::make_shared<Input>();
    window.set_input_handler(input);

    // scene settings
    glm::vec3 light_direction(1.0f, -1.0f, -1.0f);
    float wind_direction = 315.0f;
    glm::vec3 fog_color(0.9f, 0.9f, 0.9f);
    float fog_percent = 0.2f;

    // meshes data
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
         {0.5f, 0.75f, 0.4f}},
        {{0.0f, 1.0f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, 1.0f},
         {0.9f, 0.75f, 0.5f}},
        {{-0.10f, 0.8f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, 1.0f},
         {0.5f, 0.75f, 0.4f}},
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
         {0.5f, 0.75f, 0.4f}},
        {{0.0f, 1.0f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, -1.0f},
         {0.9f, 0.75f, 0.5f}},
        {{0.10f, 0.8f, 0.0f},
         {0.0f, 0.0f},
         {0.0f, 0.0f, -1.0f},
         {0.5f, 0.75f, 0.4f}},
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

    glm::vec4 cube[8] = {
        glm::vec4(1.0f, 1.0f, 1.0f, 1.0f),
        glm::vec4(-1.0f, 1.0f, 1.0f, 1.0f),
        glm::vec4(-1.0f, -1.0f, 1.0f, 1.0f),
        glm::vec4(1.0f, -1.0f, 1.0f, 1.0f),
        glm::vec4(1.0f, 1.0f, -1.0f, 1.0f),
        glm::vec4(-1.0f, 1.0f, -1.0f, 1.0f),
        glm::vec4(-1.0f, -1.0f, -1.0f, 1.0f),
        glm::vec4(1.0f, -1.0f, -1.0f, 1.0f),
    };

    std::vector<Vertex> view_frustum_vertices(8);
    for (int i = 0; i < 8; ++i) {
        glm::vec4 clip_position =
            glm::inverse(camera.get_projection()) * cube[i];
        view_frustum_vertices[i].position =
            glm::vec3(clip_position) / clip_position.w;
        view_frustum_vertices[i].color = glm::vec3(1.0f);
    }
    std::vector<int> view_frustum_indices = {
        0, 1, 1, 2, 2, 3, 3, 0, 4, 5, 5, 6, 6, 7, 7, 4, 0, 4, 1, 5, 2, 6, 3, 7};

    // load shaders
    Shader single_color;
    single_color.load_shader_from_path(
        "resources/shaders/single_color_vertex.glsl", GL_VERTEX_SHADER);
    single_color.load_shader_from_path(
        "resources/shaders/single_color_fragment.glsl", GL_FRAGMENT_SHADER);

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

    Shader flow_field;
    flow_field.load_shader_from_path("resources/shaders/sine_map.glsl",
                                     GL_COMPUTE_SHADER);

    Shader displacement;
    displacement.load_shader_from_path("resources/shaders/displacement.glsl",
                                       GL_COMPUTE_SHADER);

    // shader settings
    default_shader.set_uniform_vector3("light_direction", light_direction);
    default_shader.set_uniform_vector3("fog_color", fog_color);
    default_shader.set_uniform_float("bias", 0.4f);
    default_shader.set_uniform_float("view_distance",
                                     camera.get_far_clip_plane() - 20.0f);
    default_shader.set_uniform_float("flog_bias", fog_percent);

    gpu_instancing_shader.set_uniform_vector3("light_direction",
                                              light_direction);
    gpu_instancing_shader.set_uniform_vector3("fog_color", fog_color);
    gpu_instancing_shader.set_uniform_float("bias", 0.4f);
    gpu_instancing_shader.set_uniform_float(
        "view_distance", camera.get_far_clip_plane() - 20.0f);
    gpu_instancing_shader.set_uniform_float("flog_bias", fog_percent);
    gpu_instancing_shader.set_uniform_vector2(
        "wind_direction", glm::vec2(cos(wind_direction), sin(wind_direction)));

    flow_field.set_uniform_vector2(
        "wind_direction", glm::vec2(cos(wind_direction), sin(wind_direction)));

    // init meshes
    Mesh grass_mesh;
    grass_mesh.set(grass_vertices, grass_indices);

    Mesh frustum_mesh;
    frustum_mesh.set(view_frustum_vertices, view_frustum_indices);

    std::vector<std::shared_ptr<Chunk>> chunks;
    for (int x = -8; x < 8; ++x) {
        for (int y = -8; y < 8; ++y) {
            std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>(
                grass_mesh, field_generator_shader, glm::ivec3(x, 0, y), 2, 32);
            chunks.push_back(chunk);
        }
    }
    // textures
    RenderTexture screen_texture(window.get_size(), GL_RGB);
    screen_texture.set_filter_mode(GL_NEAREST);

    // timer
    Timer fixed_timer;
    Timer delta_timer;
    float delta_time;

    // imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    ImGui_ImplGlfw_InitForOpenGL(window.get_handler(), true);
    ImGui_ImplOpenGL3_Init("#version 430 core");
    bool open_debug_window = true;

    while (window.is_open()) {
        window.poll_events();
        if (input->is_key_down(GLFW_KEY_ESCAPE)) {
            window.close();
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        camera.set_position(glm::vec3(0.0f, 12.0f, 0.0f) +
                            glm::vec3(cos(fixed_timer.get_time() * 0.1f), 0.0f,
                                      sin(fixed_timer.get_time() * 0.1f)) *
                                20.0f);
        camera.look_at(glm::vec3(0.0f, 5.0f, 0.0f));
        camera2.look_at(glm::vec3(0.0f, 0.0f, 0.0f));

        for (std::shared_ptr<Chunk> chunk : chunks) {
            chunk->frustum_test(camera);
        }

        if (open_debug_window) {
            ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
            ImGui::SetNextWindowSize(ImVec2(640.0f, 640.0f));
            ImGui::Begin("debug", &open_debug_window,
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            ImGui::Text("grass count: %d", Chunk::grass_count);
            ImGui::Text("FPS: %.1f", 1.0f / delta_time);
            ImTextureID scene = screen_texture.get_id();
            ImGui::Image(scene,
                         ImVec2(600.0f, 600.0f / (window.get_size().x /
                                                  (float)window.get_size().y)),
                         ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
            ImGui::End();
        }

        for (std::shared_ptr<Chunk> chunk : chunks) {
            chunk->update(flow_field, displacement,
                          glm::radians(wind_direction),
                          fixed_timer.get_time() * 3.0f);
        }

        // scene view
        // screen_texture.begin_draw();
        // glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
        // glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        // renderer.set_camera(camera2);

        // default_shader.set_uniform_int("disable_fog", 1);
        // default_shader.set_uniform_vector3("camera_position",
        //                                    camera2.get_position());
        // gpu_instancing_shader.set_uniform_int("disable_fog", 1);
        // gpu_instancing_shader.set_uniform_vector3("camera_position",
        //                                           camera2.get_position());
        // for (std::shared_ptr<Chunk> chunk : chunks) {
        //     chunk->render(renderer, default_shader, gpu_instancing_shader);
        // }
        // glDisable(GL_CULL_FACE);
        // renderer.draw(frustum_mesh, camera.get_transform(), single_color,
        //               GL_LINES);
        // glEnable(GL_CULL_FACE);

        // screen_texture.end_draw();

        // debug view
        glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.set_camera(camera);

        default_shader.set_uniform_int("disable_fog", 0);
        default_shader.set_uniform_vector3("camera_position",
                                           camera.get_position());
        gpu_instancing_shader.set_uniform_int("disable_fog", 0);
        gpu_instancing_shader.set_uniform_vector3("camera_position",
                                                  camera.get_position());
        for (std::shared_ptr<Chunk> chunk : chunks) {
            chunk->render(renderer, default_shader, gpu_instancing_shader);
        }

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