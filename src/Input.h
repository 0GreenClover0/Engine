#pragma once

#include <functional>

#include "Window.h"

class Input
{
public:
    static void set_input(std::shared_ptr<Input> const& input_system);

    explicit Input(std::shared_ptr<Window> const& window);
    [[nodiscard]] bool is_key_down(int const key) const;

    // TODO: Handle custom callbacks using an observer pattern
    static void mouse_callback(GLFWwindow* window, double const x, double const y);
    std::function<void(double const, double const)> mouse_callback_impl;

    static void focus_callback(GLFWwindow* window, int const focused);
    std::function<void(int const)> focus_callback_impl;

    inline static std::shared_ptr<Input> input;
private:
    std::shared_ptr<Window> window;

};
