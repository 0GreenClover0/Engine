#pragma once

#include "GLFW/glfw3.h"

class Window
{
public:
    Window(int32_t screen_width, int32_t screen_height, int32_t const subsamples = 0);
    Window() = delete;

    [[nodiscard]] GLFWwindow* get_glfw_window() const;

private:
    GLFWwindow* window;
};
