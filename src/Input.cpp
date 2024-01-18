#include "Input.h"

void Input::set_input(std::shared_ptr<Input> const& input_system)
{
    input = input_system;

    glfwSetCursorPosCallback(input->window->get_glfw_window(), &input->mouse_callback);
    glfwSetWindowFocusCallback(input->window->get_glfw_window(), &input->focus_callback);
}

Input::Input(std::shared_ptr<Window> const& window) : window(window)
{
    keys.insert(std::pair<int32_t, Key>(GLFW_KEY_W, GLFW_KEY_W));
    keys.insert(std::pair<int32_t, Key>(GLFW_KEY_S, GLFW_KEY_S));
    keys.insert(std::pair<int32_t, Key>(GLFW_KEY_A, GLFW_KEY_A));
    keys.insert(std::pair<int32_t, Key>(GLFW_KEY_D, GLFW_KEY_D));
    keys.insert(std::pair<int32_t, Key>(GLFW_KEY_Q, GLFW_KEY_Q));
    keys.insert(std::pair<int32_t, Key>(GLFW_KEY_E, GLFW_KEY_E));
    keys.insert(std::pair<int32_t, Key>(GLFW_KEY_T, GLFW_KEY_T));
}

bool Input::get_key(int32_t const key) const
{
    return keys.at(key).get_key();
}

bool Input::get_key_down(int32_t const key) const
{
    return keys.at(key).get_key_down();
}

void Input::mouse_callback(GLFWwindow* window, double const x, double const y)
{
    input->on_set_cursor_pos_event(x, y);
}

void Input::focus_callback(GLFWwindow* window, int const focused)
{
    input->on_focused_event(focused);
}

void Input::update_keys()
{
    for (auto& [number, key] : keys)
    {
        key.was_down_last_frame = key.is_down_this_frame;
        key.is_down_this_frame = is_key_pressed(key.key);
    }
}

bool Input::is_key_pressed(int const key) const
{
    return glfwGetKey(window->get_glfw_window(), key) == GLFW_PRESS;
}
