#pragma once

#include <map>

#include "Event.h"
#include "Key.h"
#include "Window.h"

class Input
{
public:
    static void set_input(std::shared_ptr<Input> const& input_system);

    explicit Input(std::shared_ptr<Window> const& window);
    [[nodiscard]] bool get_key(i32 const key) const;
    [[nodiscard]] bool get_key_down(i32 const key) const;

    // TODO: Handle custom callbacks using an observer pattern
    static void mouse_callback(GLFWwindow* window, double const x, double const y);

    static void focus_callback(GLFWwindow* window, i32 const focused);

    inline static std::shared_ptr<Input> input;

    void update_keys();

    Event<void(i32 const)> on_focused_event;
    Event<void(double const, double const)> on_set_cursor_pos_event;
private:
    [[nodiscard]] bool is_key_pressed(i32 const key) const;

    std::map<i32, Key> m_keys = {};

    std::shared_ptr<Window> m_window;
};
