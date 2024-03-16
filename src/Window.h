#pragma once

#include <d3d11.h>

#include "AK/Types.h"
#include "GLFW/glfw3.h"
#include "Renderer.h"

class Window
{
public:
    Window(Renderer::RendererApi const renderer_api, i32 const screen_width, i32 const screen_height, i32 const subsamples = 0, bool const enable_vsync = false, bool const enable_mouse_capture = true);
    Window() = delete;

    [[nodiscard]] GLFWwindow* get_glfw_window() const;
    [[nodiscard]] HWND get_win32_window() const;

private:
    GLFWwindow* m_window;
};
