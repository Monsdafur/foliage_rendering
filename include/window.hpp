#pragma once

#include "GLFW/glfw3.h"
#include "glm/ext/vector_float2.hpp"
#include "glm/ext/vector_int2.hpp"
#include <map>
#include <memory>
#include <string>

class Timer {
  public:
    Timer() = default;

    double get_time();

    double reset();

    void set_pause(bool flag);

  private:
    double m_start = 0.0f;
    double m_pause_time = 0.0f;
    bool m_paused = false;
};

class Input {
  public:
    void clear_inputs();

    bool is_key_pressed(GLuint key);

    bool is_key_down(GLuint key);

    bool is_key_up(GLuint key);

    bool is_mouse_pressed(GLuint button);

    bool is_mouse_down(GLuint button);

    bool is_mouse_up(GLuint button);

    void set_key_pressed(GLuint key, bool flag);

    void set_key_down(GLuint key, bool flag);

    void set_key_up(GLuint key, bool flag);

    void set_mouse_pressed(GLuint button, bool flag);

    void set_mouse_down(GLuint button, bool flag);

    void set_mouse_up(GLuint button, bool flag);

  private:
    std::map<GLuint, bool> m_key_press;
    std::map<GLuint, bool> m_key_down;
    std::map<GLuint, bool> m_key_up;

    std::map<GLuint, bool> m_mouse_press;
    std::map<GLuint, bool> m_mouse_down;
    std::map<GLuint, bool> m_mouse_up;
};

class Window {
  public:
    friend class Event;
    Window(const glm::ivec2& size, const std::string& title);

    ~Window();

    bool is_open();

    GLFWwindow* get_handler() const;

    glm::vec2 get_mouse_position();

    glm::vec2 get_mouse_position_normalized();

    glm::ivec2 get_size() const;

    void set_input_handler(std::shared_ptr<Input> input);

    void poll_events();

    void close();

    void display();

  private:
    static void glfw_key_callback(GLFWwindow* window, int key, int scancode,
                                  int action, int mod);

    static void glfw_mouse_callback(GLFWwindow* window, int button, int action,
                                    int mods);

    glm::ivec2 m_size;

    GLFWwindow* m_handler;

    std::shared_ptr<Input> m_input;
    glm::vec2 m_mouse_position;

    static bool m_glfw_initialized;
    static int m_active_windows;
};