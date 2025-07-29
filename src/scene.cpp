#include "scene.hpp"
#include <fstream>
#include <sstream>

static Mesh load_model(const std::filesystem::path& model_path) {
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
        printf("%.1f %.1f %.1f\n", vertex.position.x, vertex.position.y,
               vertex.position.z);

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

Scene::Scene(const std::filesystem::path& shader_directory,
             const std::filesystem::path& model_directory, Settings& settings,
             Camera& camera)
    : m_settings(settings), m_camera(camera) {
    m_default_shader.load_shader_from_path(
        shader_directory / "default_vertex.glsl", GL_VERTEX_SHADER);
    m_default_shader.load_shader_from_path(
        shader_directory / "default_fragment.glsl", GL_FRAGMENT_SHADER);
    m_post_processing.load_shader_from_path(
        shader_directory / "screen_vertex.glsl", GL_VERTEX_SHADER);
    m_post_processing.load_shader_from_path(
        shader_directory / "post_processing.glsl", GL_FRAGMENT_SHADER);
    m_gpu_instancing_shader.load_shader_from_path(
        shader_directory / "gpu_instancing.glsl", GL_VERTEX_SHADER);
    m_gpu_instancing_shader.load_shader_from_path(
        shader_directory / "default_fragment.glsl", GL_FRAGMENT_SHADER);
    m_grass_generation_shader.load_shader_from_path(
        shader_directory / "grass_generation.glsl", GL_COMPUTE_SHADER);
    m_flow_field.load_shader_from_path(shader_directory / "flow_field.glsl",
                                       GL_COMPUTE_SHADER);
    m_displacement.load_shader_from_path(shader_directory / "displacement.glsl",
                                         GL_COMPUTE_SHADER);

    m_default_shader.set_uniform_vector3("light_direction",
                                         m_settings.light_direction);
    m_default_shader.set_uniform_vector3("fog_color", m_settings.fog_color);
    m_default_shader.set_uniform_float("bias", 0.6f);
    m_default_shader.set_uniform_float("view_distance",
                                       m_camera.get_far_clip_plane());
    m_default_shader.set_uniform_float("flog_bias", m_settings.fog_percent);

    m_gpu_instancing_shader.set_uniform_vector3("light_direction",
                                                m_settings.light_direction);
    m_gpu_instancing_shader.set_uniform_vector3("fog_color",
                                                m_settings.fog_color);
    m_gpu_instancing_shader.set_uniform_float("bias", 0.6f);
    m_gpu_instancing_shader.set_uniform_float("view_distance",
                                              m_camera.get_far_clip_plane());
    m_gpu_instancing_shader.set_uniform_vector2(
        "wind_direction", glm::vec2(cos(m_settings.wind_direction),
                                    sin(m_settings.wind_direction)));

    m_flow_field.set_uniform_vector2("wind_direction",
                                     glm::vec2(cos(m_settings.wind_direction),
                                               sin(m_settings.wind_direction)));

    m_grass_mesh = load_model(model_directory / "grass_model.txt");

    for (int x = -8; x < 8; ++x) {
        for (int y = -8; y < 8; ++y) {
            std::shared_ptr<Chunk> chunk = std::make_shared<Chunk>(
                m_grass_mesh, m_grass_generation_shader, glm::ivec3(x, 0, y), 2,
                32, 34.0f, 0.01f, 0u);
            m_chunks.push_back(chunk);
        }
    }
}

void Scene::update(float time) {
    for (std::shared_ptr<Chunk> chunk : m_chunks) {
        chunk->frustum_test(m_camera);
        chunk->update(m_flow_field, m_displacement,
                      glm::radians(m_settings.wind_direction), time * 6.0f);
    }
}

void Scene::render(Renderer& renderer, const Camera& external_camera) {
    renderer.set_camera(external_camera);
    m_default_shader.set_uniform_float("fog_bias", m_settings.fog_percent);
    m_gpu_instancing_shader.set_uniform_float("fog_bias",
                                              m_settings.fog_percent);
    m_default_shader.set_uniform_int("disable_fog", 0);
    m_default_shader.set_uniform_vector3("camera_position",
                                         m_camera.get_position());
    m_gpu_instancing_shader.set_uniform_int("disable_fog", 0);
    m_gpu_instancing_shader.set_uniform_vector3("camera_position",
                                                m_camera.get_position());
    for (std::shared_ptr<Chunk> chunk : m_chunks) {
        chunk->render(renderer, m_default_shader, m_gpu_instancing_shader);
    }
}
