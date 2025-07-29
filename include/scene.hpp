#pragma once

#include "chunk.hpp"
#include "glm/trigonometric.hpp"
#include "renderer.hpp"
#include <filesystem>
#include <memory>

struct Settings {
    glm::vec3 light_direction =
        glm::vec3(cos(glm::radians(135.0f)), -0.5f, sin(glm::radians(135.0f)));
    float wind_direction = 315.0f;
    glm::vec3 fog_color = glm::vec3(0.9f, 0.9f, 0.9f);
    float fog_percent = 0.0f;
    float distance = 10.0f;
    float angle = 0.0f;
    float height = 34.0f;
    bool auto_rotate = false;
    bool show_debug_view = false;
};

class Scene {
  public:
    Scene(const std::filesystem::path& shader_directory,
          const std::filesystem::path& model_directory, Settings& settings,
          Camera& camera);

    void update(float time);

    void render(Renderer& renderer, const Camera& external_camera);

  private:
    Settings& m_settings;
    Camera& m_camera;
    Shader m_single_color;
    Shader m_default_shader;
    Shader m_post_processing;
    Shader m_gpu_instancing_shader;
    Shader m_grass_generation_shader;
    Shader m_flow_field;
    Shader m_displacement;
    std::vector<std::shared_ptr<Chunk>> m_chunks;
    Mesh m_grass_mesh;
};