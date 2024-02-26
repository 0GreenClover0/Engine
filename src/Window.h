#pragma once

#include "AK/Types.h"
#include "GLFW/glfw3.h"

class Window
{
public:
    Window(i32 screen_width, i32 screen_height, i32 const subsamples = 0);
    Window() = delete;

    [[nodiscard]] GLFWwindow* get_glfw_window() const;

private:
    GLFWwindow* m_window;
};
