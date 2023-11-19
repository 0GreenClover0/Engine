#pragma once
#include <memory>

#include "GLFW/glfw3.h"

class Window
{
public:
    Window(int screen_width, int screen_height);
    Window() = delete;

    GLFWwindow* get_glfw_window() const;

private:
    GLFWwindow* window;
};
