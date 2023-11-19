#include "Window.h"

#include <stdexcept>

#include "imgui_impl/imgui_impl_opengl3_loader.h"

Window::Window(int const screen_width, int const screen_height)
{
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

    window = glfwCreateWindow(screen_width, screen_height, "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);

    if (window == nullptr)
        throw std::runtime_error("Could not create a window.");

    glfwMakeContextCurrent(window);
}

GLFWwindow* Window::get_glfw_window() const
{
    return window;
}
