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
#include "texture.hpp"
#include "window.hpp"
#include <fstream>
#include <memory>
#include <sstream>

Mesh load_model(const std::filesystem::path& model_path) {
    std::fstream file(model_path);
    std::string line;
    std::string word;
    std::stringstream stream;

    std::vector<Vertex> vertices;
    std::getline(file, line);
    int vertex_count = std::stoi(line);

    for (int i = 0; i < vertex_count; ++i) {
        Vertex vertex;

        // position
        std::getline(file, line);
        stream = std::stringstream(line);
        stream >> word;
        vertex.position.x = std::stof(word);
        stream >> word;
        vertex.position.y = std::stof(word);
        stream >> word;
        vertex.position.z = std::stof(word);

        // uv
        std::getline(file, line);
        stream = std::stringstream(line);
        stream >> word;
        vertex.uv.x = std::stof(word);
        stream >> word;
        vertex.uv.y = std::stof(word);

        // normal
        std::getline(file, line);
        stream = std::stringstream(line);
        stream >> word;
        vertex.normal.x = std::stof(word);
        stream >> word;
        vertex.normal.y = std::stof(word);
        stream >> word;
        vertex.normal.z = std::stof(word);

        // color
        std::getline(file, line);
        stream = std::stringstream(line);
        stream >> word;
        vertex.color.x = std::stof(word);
        stream >> word;
        vertex.color.y = std::stof(word);
        stream >> word;
        vertex.color.z = std::stof(word);

        vertices.push_back(vertex);
    }

    std::vector<int> indices;
    std::getline(file, line);
    int index_count = std::stoi(line);

    for (int i = 0; i < index_count; ++i) {
        std::getline(file, line);
        indices.push_back(std::stoi(line));
    }

    file.close();

    Mesh mesh;
    mesh.set(vertices, indices);

    return mesh;
}

int main() {
    // init window
    Window window(glm::ivec2(1920, 1080), "grass field");
    Renderer renderer;
    Camera camera(glm::vec3(0.0f, 0.0f, 0.0f), glm::radians(45.0f),
                  (float)window.get_size().x / (float)window.get_size().y, 0.1f,
                  200.0f);

    Camera camera2(glm::vec3(100.0f, 200.0f, 100.0f), window.get_size() / 4,
                   0.1f, 400.0f);

    renderer.set_camera(camera);
    glViewport(0, 0, window.get_size().x, window.get_size().y);

    std::shared_ptr<Input> input = std::make_shared<Input>();
    window.set_input_handler(input);

    // scene settings
    glm::vec3 light_direction(cos(glm::radians(135.0f)), -0.5f,
                              sin(glm::radians(135.0f)));
    float wind_direction = 315.0f;
    glm::vec3 fog_color(0.9f, 0.9f, 0.9f);
    float fog_percent = 0.0f;
    float distance = 10.0f;
    float angle = 0.0f;
    float height = 34.0f;
    bool auto_rotate = false;
    bool show_debug_view = false;

    // meshes data
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

    Shader grass_generation_shader;
    grass_generation_shader.load_shader_from_path(
        "resources/shaders/grass_generation.glsl", GL_COMPUTE_SHADER);

    Shader flow_field;
    flow_field.load_shader_from_path("resources/shaders/flow_field.glsl",
                                     GL_COMPUTE_SHADER);

    Shader displacement;
    displacement.load_shader_from_path("resources/shaders/displacement.glsl",
                                       GL_COMPUTE_SHADER);

    // shader settings
    default_shader.set_uniform_vector3("light_direction", light_direction);
    default_shader.set_uniform_vector3("fog_color", fog_color);
    default_shader.set_uniform_float("bias", 0.6f);
    default_shader.set_uniform_float("view_distance",
                                     camera.get_far_clip_plane());
    default_shader.set_uniform_float("flog_bias", fog_percent);

    gpu_instancing_shader.set_uniform_vector3("light_direction",
                                              light_direction);
    gpu_instancing_shader.set_uniform_vector3("fog_color", fog_color);
    gpu_instancing_shader.set_uniform_float("bias", 0.6f);
    gpu_instancing_shader.set_uniform_float("view_distance",
                                            camera.get_far_clip_plane());
    gpu_instancing_shader.set_uniform_vector2(
        "wind_direction", glm::vec2(cos(wind_direction), sin(wind_direction)));

    flow_field.set_uniform_vector2(
        "wind_direction", glm::vec2(cos(wind_direction), sin(wind_direction)));

    // init meshes
    Mesh grass_mesh = load_model("resources/models/grass_model.txt");
    Mesh screen_mesh;
    screen_mesh.set(screen_vertices, screen_indices);

    Mesh frustum_mesh;
    frustum_mesh.set(view_frustum_vertices, view_frustum_indices);

    std::vector<std::shared_ptr<Chunk>> chunks;
    for (int x = -8; x < 8; ++x) {
        for (int y = -8; y < 8; ++y) {
            std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>(
                grass_mesh, grass_generation_shader, glm::ivec3(x, 0, y), 2, 32,
                34.0f, 0.01f, 0u);
            chunks.push_back(chunk);
        }
    }
    // textures
    std::shared_ptr<RenderTexture> screen_texture =
        std::make_shared<RenderTexture>(window.get_size(), GL_RGB);
    screen_texture->set_filter_mode(GL_NEAREST);
    std::shared_ptr<RenderTexture> post_processing_texture =
        std::make_shared<RenderTexture>(window.get_size(), GL_RGB);
    post_processing_texture->set_filter_mode(GL_NEAREST);
    screen_mesh.set_texture(post_processing_texture);

    // timer
    Timer fixed_timer;
    Timer delta_timer;
    float delta_time;
    float fps_update_interval = 0.5f;
    float next_fps_update;
    int fps = 60;

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
    ImGuiStyle& style = ImGui::GetStyle();
    style.Colors[ImGuiCol_WindowBg].w = 0.4f;

    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    while (window.is_open()) {
        window.poll_events();
        if (input->is_key_down(GLFW_KEY_ESCAPE)) {
            window.close();
        }

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        camera.set_position(glm::vec3(0.0f, height, 0.0f) +
                            glm::vec3(cos(glm::radians(angle)), 0.0f,
                                      sin(glm::radians(angle))) *
                                distance);
        camera.look_at(glm::vec3(0.0f, height, 0.0f));
        camera2.look_at(glm::vec3(0.0f, 0.0f, 0.0f));

        for (std::shared_ptr<Chunk> chunk : chunks) {
            chunk->frustum_test(camera);
        }

        angle += auto_rotate * 10.0f * delta_time;
        if (angle > 360.0f) {
            angle = angle - 360.0f;
        }

        default_shader.set_uniform_float("fog_bias", fog_percent);
        gpu_instancing_shader.set_uniform_float("fog_bias", fog_percent);

        if (open_debug_window) {
            if (fixed_timer.get_time() > next_fps_update) {
                next_fps_update = fixed_timer.get_time() + fps_update_interval;
                fps = 1.0f / delta_time;
            }

            ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
            ImGui::SetNextWindowSize(ImVec2(532.0f, window.get_size().y));
            ImGui::Begin("debug", &open_debug_window,
                         ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);
            ImGui::Text("grass count: %d", Chunk::grass_count);
            ImGui::Text("FPS: %d", fps);
            ImGui::SliderFloat("angle", &angle, 0.0f, 360.0f);
            ImGui::SliderFloat("distance", &distance, 5.0f, 32.0f * 8.0f);
            ImGui::SliderFloat("height", &height, 0.0f, 60.0f);
            ImGui::Checkbox("auto rotate", &auto_rotate);
            if (ImGui::Checkbox("show debug view", &show_debug_view)) {
                if (show_debug_view) {
                    screen_mesh.set_texture(screen_texture);
                } else {
                    screen_mesh.set_texture(post_processing_texture);
                }
            }
            // if (show_debug_view) {
            //     ImTextureID scene = screen_texture.get_id();
            //     ImGui::Text("debug view");
            //     ImGui::Image(scene,
            //                  ImVec2(screen_texture.get_size().x,
            //                         screen_texture.get_size().y),
            //                  ImVec2(0.0f, 1.0f), ImVec2(1.0f, 0.0f));
            // }
            ImGui::End();
        }

        for (std::shared_ptr<Chunk> chunk : chunks) {
            chunk->update(flow_field, displacement,
                          glm::radians(wind_direction),
                          fixed_timer.get_time() * 6.0f);
        }

        // debug view
        if (show_debug_view) {
            screen_texture->begin_draw();
            glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glViewport(0, 0, screen_texture->get_size().x,
                       screen_texture->get_size().y);
            renderer.set_camera(camera2);

            default_shader.set_uniform_int("disable_fog", 1);
            default_shader.set_uniform_vector3("camera_position",
                                               camera.get_position());
            gpu_instancing_shader.set_uniform_int("disable_fog", 1);
            gpu_instancing_shader.set_uniform_vector3("camera_position",
                                                      camera.get_position());
            for (std::shared_ptr<Chunk> chunk : chunks) {
                chunk->render(renderer, default_shader, gpu_instancing_shader,
                              true);
            }
            glDisable(GL_CULL_FACE);
            renderer.draw(frustum_mesh, camera.get_transform(), single_color,
                          GL_LINES);
            glEnable(GL_CULL_FACE);
            screen_texture->end_draw();
            glViewport(0, 0, window.get_size().x, window.get_size().y);
        } else {

            // scene view
            post_processing_texture->begin_draw();
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
            post_processing_texture->end_draw();
        }
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.draw(screen_mesh, glm::mat4(1.0f), post_processing);

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