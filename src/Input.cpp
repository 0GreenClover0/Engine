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
