#include "window.hpp"
#include <iostream>
#include <memory>

double Timer::get_time() {
    if (!m_paused) {
        return glfwGetTime() - m_start;
    } else {
        return m_pause_time - m_start;
    }
}

double Timer::reset() {
    double time = get_time();
    m_start = glfwGetTime();
    return time;
}

void Timer::set_pause(bool flag) {
    if (flag != m_paused) {
        if (flag) {
            m_pause_time = glfwGetTime();
        } else {
            m_start += glfwGetTime() - m_pause_time;
        }
    }

    m_paused = flag;
}

// input

void Input::clear_inputs() {
    for (auto it = m_key_down.begin(); it != m_key_down.end(); ++it) {
        it->second = false;
    }
    for (auto it = m_key_up.begin(); it != m_key_up.end(); ++it) {
        it->second = false;
    }
    for (auto it = m_mouse_down.begin(); it != m_mouse_down.end(); ++it) {
        it->second = false;
    }
    for (auto it = m_mouse_up.begin(); it != m_mouse_up.end(); ++it) {
        it->second = false;
    }
}

bool Input::is_key_pressed(GLuint key) { return m_key_press[key]; }

bool Input::is_key_down(GLuint key) { return m_key_down[key]; }

bool Input::is_key_up(GLuint key) { return m_key_up[key]; }

bool Input::is_mouse_pressed(GLuint button) { return m_mouse_press[button]; }

bool Input::is_mouse_down(GLuint button) { return m_mouse_down[button]; }

bool Input::is_mouse_up(GLuint button) { return m_mouse_up[button]; }

void Input::set_key_pressed(GLuint key, bool flag) { m_key_press[key] = flag; }

void Input::set_key_down(GLuint key, bool flag) { m_key_down[key] = flag; }

void Input::set_key_up(GLuint key, bool flag) { m_key_up[key] = flag; }

void Input::set_mouse_pressed(GLuint button, bool flag) {
    m_mouse_press[button] = flag;
}

void Input::set_mouse_down(GLuint button, bool flag) {
    m_mouse_down[button] = flag;
}

void Input::set_mouse_up(GLuint button, bool flag) {
    m_mouse_up[button] = flag;
}

// window
bool Window::m_glfw_initialized = false;
int Window::m_active_windows = 0;

Window::Window(const glm::ivec2& size, const std::string& title) {
    if (!m_glfw_initialized) {
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        // glfwWindowHint(GLFW_SAMPLES, 4);
        glfwSwapInterval(1);
        m_glfw_initialized = true;
    }
    m_handler = glfwCreateWindow(size.x, size.y, title.c_str(), 0, 0);
    if (m_handler == 0) {
        std::cout << "Failed to create window" << std::endl;
    } else {
        m_size = size;
        m_active_windows++;
        glfwMakeContextCurrent(m_handler);
        glfwSetKeyCallback(m_handler, glfw_key_callback);
        glfwSetMouseButtonCallback(m_handler, glfw_mouse_callback);
        glfwSetWindowUserPointer(m_handler, this);
    }
}

Window::~Window() {
    glfwDestroyWindow(m_handler);
    m_active_windows--;
    if (!m_active_windows) {
        glfwTerminate();
    }
}

bool Window::is_open() { return !glfwWindowShouldClose(m_handler); }

GLFWwindow* Window::get_handler() const { return m_handler; }

glm::vec2 Window::get_mouse_position() { return m_mouse_position; }

glm::vec2 Window::get_mouse_position_normalized() {
    glm::vec2 normalized = m_mouse_position;
    normalized.x = normalized.x / (m_size.x * 0.5f) - 1.0f;
    normalized.y = (2.0f - normalized.y / (m_size.y * 0.5f)) - 1.0f;
    return normalized;
}

glm::ivec2 Window::get_size() const { return m_size; }

void Window::set_input_handler(std::shared_ptr<Input> input) {
    m_input = input;
}

void Window::poll_events() {
    m_input->clear_inputs();
    glfwWaitEventsTimeout(1.0f / 360.0f);

    double x = 0.0f;
    double y = 0.0f;
    glfwGetCursorPos(m_handler, &x, &y);
    m_mouse_position = glm::vec2(x, y);
}

void Window::close() { glfwSetWindowShouldClose(m_handler, true); }

void Window::display() { glfwSwapBuffers(m_handler); }

void Window::glfw_key_callback(GLFWwindow* window, int key, int scancode,
                               int action, int mod) {
    Window* wrapper = (Window*)glfwGetWindowUserPointer(window);

    if (action != GLFW_RELEASE) {
        wrapper->m_input->set_key_pressed(key, action == GLFW_REPEAT ||
                                                   action == GLFW_PRESS);
    } else {
        wrapper->m_input->set_key_pressed(key, false);
    }

    wrapper->m_input->set_key_down(key, action == GLFW_PRESS);
    wrapper->m_input->set_key_up(key, action == GLFW_RELEASE);
}

void Window::glfw_mouse_callback(GLFWwindow* window, int button, int action,
                                 int mod) {
    Window* wrapper = (Window*)glfwGetWindowUserPointer(window);

    if (action != GLFW_RELEASE) {
        wrapper->m_input->set_mouse_pressed(button, action == GLFW_REPEAT ||
                                                        action == GLFW_PRESS);
    } else {
        wrapper->m_input->set_mouse_pressed(button, false);
    }

    wrapper->m_input->set_mouse_down(button, action == GLFW_PRESS);
    wrapper->m_input->set_mouse_up(button, action == GLFW_RELEASE);
}