#include "texture.hpp"
#include "glad/glad.h"
#include "stb_image.h"
#include "utility.hpp"
#include <iostream>

Texture::Texture() {
    glGenTextures(1, &m_id);
    glBindTexture(GL_TEXTURE_2D, m_id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glBindTexture(GL_TEXTURE_2D, 0);
}

Texture::~Texture() { glDeleteTextures(1, &m_id); }

void Texture::load_texture_from_byte(uint8_t* pixel_data, GLuint type,
                                     const glm::ivec2& size,
                                     GLuint internal_format, GLuint format) {
    m_size = size;
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, m_size.x, m_size.y, 0,
                 format, type, pixel_data);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::load_texture_from_path(
    const std::filesystem::path& texture_path) {
    stbi_set_flip_vertically_on_load(true);
    glm::ivec2 size;
    uint8_t* pixel_data =
        stbi_load(texture_path.generic_string().c_str(), &size.x, &size.y,
                  &m_color_channel_count, 0);
    if (!pixel_data) {
        std::cerr << "FAILED TO LOAD IMAGE AT PATH: " << texture_path
                  << std::endl;
    }

    GLuint format;
    switch (m_color_channel_count) {
    case 1:
        format = GL_RED;
        break;
    case 3:
        format = GL_RGB;
        break;
    case 4:
        format = GL_RGBA;
        break;
    }

    load_texture_from_byte(pixel_data, GL_UNSIGNED_BYTE, size, format, format);
    glBindTexture(GL_TEXTURE_2D, m_id);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    stbi_image_free(pixel_data);
}

void Texture::set_filter_mode(GLuint mode) {
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mode);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::set_wrap_mode(GLuint mode) {
    glBindTexture(GL_TEXTURE_2D, m_id);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, mode);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, mode);
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint Texture::get_id() const { return m_id; }

glm::ivec2 Texture::get_size() const { return m_size; }

// render texture
RenderTexture::RenderTexture(const glm::ivec2& size, GLuint format)
    : Texture() {
    glGenFramebuffers(1, &m_frame_buffer);
    gl_check_error();
    glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer);
    gl_check_error();

    glGenRenderbuffers(1, &m_render_buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, m_render_buffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, size.x, size.y);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    load_texture_from_byte(0, GL_UNSIGNED_BYTE, size, format, format);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                           m_id, 0);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, m_render_buffer);

    gl_check_error();

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR WHILE INITIALIZING FRAMEBUFFER" << std::endl;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

RenderTexture::~RenderTexture() {
    glDeleteTextures(1, &m_id);
    glDeleteFramebuffers(1, &m_frame_buffer);
}

void RenderTexture::begin_draw() {
    glBindFramebuffer(GL_FRAMEBUFFER, m_frame_buffer);
}

void RenderTexture::end_draw() { glBindFramebuffer(GL_FRAMEBUFFER, 0); }