#include "Input.h"

void Input::set_input(std::shared_ptr<Input> const& input_system)
{
    input = input_system;

    glfwSetCursorPosCallback(input->m_window->get_glfw_window(), &input->mouse_callback);
    glfwSetWindowFocusCallback(input->m_window->get_glfw_window(), &input->focus_callback);
}

Input::Input(std::shared_ptr<Window> const& window) : m_window(window)
{
    m_keys.insert(std::pair<i32, Key>(GLFW_KEY_W, GLFW_KEY_W));
    m_keys.insert(std::pair<i32, Key>(GLFW_KEY_S, GLFW_KEY_S));
    m_keys.insert(std::pair<i32, Key>(GLFW_KEY_A, GLFW_KEY_A));
    m_keys.insert(std::pair<i32, Key>(GLFW_KEY_D, GLFW_KEY_D));
    m_keys.insert(std::pair<i32, Key>(GLFW_KEY_Q, GLFW_KEY_Q));
    m_keys.insert(std::pair<i32, Key>(GLFW_KEY_E, GLFW_KEY_E));
    m_keys.insert(std::pair<i32, Key>(GLFW_KEY_P, GLFW_KEY_P));
    m_keys.insert(std::pair<i32, Key>(GLFW_KEY_T, GLFW_KEY_T));
    m_keys.insert(std::pair<i32, Key>(GLFW_KEY_SPACE, GLFW_KEY_SPACE));
    m_keys.insert(std::pair<i32, Key>(GLFW_KEY_R, GLFW_KEY_R));
    m_keys.insert(std::pair<i32, Key>(GLFW_KEY_E, GLFW_KEY_E));
    m_keys.insert(std::pair<i32, Key>(GLFW_KEY_G, GLFW_KEY_G));
    m_keys.insert(std::pair<i32, Key>(GLFW_MOUSE_BUTTON_LEFT, GLFW_MOUSE_BUTTON_LEFT));
    m_keys.insert(std::pair<i32, Key>(GLFW_MOUSE_BUTTON_RIGHT, GLFW_MOUSE_BUTTON_RIGHT));
    m_keys.insert(std::pair<i32, Key>(GLFW_KEY_F1, GLFW_KEY_F1));
    m_keys.insert(std::pair<i32, Key>(GLFW_KEY_F2, GLFW_KEY_F2));
    m_keys.insert(std::pair<i32, Key>(GLFW_KEY_F3, GLFW_KEY_F3));
    m_keys.insert(std::pair<i32, Key>(GLFW_KEY_F4, GLFW_KEY_F4));
    m_keys.insert(std::pair<i32, Key>(GLFW_KEY_F5, GLFW_KEY_F5));
    m_keys.insert(std::pair<i32, Key>(GLFW_KEY_F6, GLFW_KEY_F6));
    m_keys.insert(std::pair<i32, Key>(GLFW_KEY_DELETE, GLFW_KEY_DELETE));
    m_keys.insert(std::pair<i32, Key>(GLFW_KEY_BACKSLASH, GLFW_KEY_BACKSLASH));
}

bool Input::get_key(i32 const key) const
{
    return m_keys.at(key).get_key();
}

bool Input::get_key_down(i32 const key) const
{
    return m_keys.at(key).get_key_down();
}

void Input::mouse_callback(GLFWwindow* window, double const x, double const y)
{
    input->on_set_cursor_pos_event(x, y);
}

void Input::focus_callback(GLFWwindow* window, i32 const focused)
{
    input->on_focused_event(focused);
}

void Input::update_keys()
{
    for (auto& [number, key] : m_keys)
    {
        key.m_was_down_last_frame = key.m_is_down_this_frame;
        key.m_is_down_this_frame = is_key_pressed(key.key);
    }
}

bool Input::is_key_pressed(i32 const key) const
{
    if (key < 8)
        return glfwGetMouseButton(m_window->get_glfw_window(), key) == GLFW_PRESS;

    return glfwGetKey(m_window->get_glfw_window(), key) == GLFW_PRESS;
}

glm::vec2 Input::get_mouse_position() const
{
    double x_position, y_position;
    glfwGetCursorPos(m_window->get_glfw_window(), &x_position, &y_position);

    i32 width, height;
    glfwGetWindowSize(m_window->get_glfw_window(), &width, &height);

    if (width == 0 || height == 0)
    {
        return {0.0f, 0.0f};
    }

    return glm::vec2((x_position / width) - 0.5f, (y_position / height) - 0.5f) * 2.0f;
}
