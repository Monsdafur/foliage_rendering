#pragma once

#include "glad/glad.h"
#include "glm/ext/vector_int2.hpp"
#include <cstdint>
#include <filesystem>

class Texture {
  public:
    Texture();

    ~Texture();

    void load_texture_from_byte(uint8_t* pixel_data, GLuint type,
                                const glm::ivec2& size, GLuint internal_format,
                                GLuint format);

    void load_texture_from_path(const std::filesystem::path& texture_path);

    void set_filter_mode(GLuint mode);

    void set_wrap_mode(GLuint mode);

    glm::ivec2 get_size() const;

    GLuint get_id() const;

  protected:
    GLuint m_id;
    glm::ivec2 m_size;
    int m_color_channel_count;
};

class RenderTexture : public Texture {
  public:
    RenderTexture(const glm::ivec2& size, GLuint format);

    ~RenderTexture();

    void begin_draw();

    void end_draw();

  private:
    GLuint m_frame_buffer;
};