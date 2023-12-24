#pragma once

#include <functional>

#include "Event.h"
#include "Window.h"

class Input
{
public:
    static void set_input(std::shared_ptr<Input> const& input_system);

    explicit Input(std::shared_ptr<Window> const& window);
    [[nodiscard]] bool is_key_down(int const key) const;

    // TODO: Handle custom callbacks using an observer pattern
    static void mouse_callback(GLFWwindow* window, double const x, double const y);

    static void focus_callback(GLFWwindow* window, int const focused);

    inline static std::shared_ptr<Input> input;

    Event<void(int const)> on_focused_event;
    Event<void(double const, double const)> on_set_cursor_pos_event;
private:

    std::shared_ptr<Window> window;

};
