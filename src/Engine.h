#pragma once

#include <imgui.h>
#include <imgui_impl/imgui_impl_glfw.h>
#include <imgui_impl/imgui_impl_opengl3.h>

#include <glad/glad.h> // Initialize with gladLoadGL()

#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions

#include <spdlog/spdlog.h>

#include <miniaudio.h>

#include <memory>

#include "Window.h"

class Engine
{
public:
    static int32_t initialize();
    static void create_game();
    static void run();
    static void clean_up();

    inline static bool enable_vsync = false;
    inline static bool enable_mouse_capture = true;

    inline static int32_t screen_width = 1280;
    inline static int32_t screen_height = 720;

    inline static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    inline static ma_engine audio_engine;

private:
    static int32_t initialize_thirdparty();

    static int32_t setup_glfw();
    static std::shared_ptr<Window> create_window();
    static void glfw_error_callback(int const error, char const* description);

    static int32_t setup_glad();

    static void setup_imgui(GLFWwindow* glfw_window);

    static int32_t setup_miniaudio();

    inline static std::shared_ptr<Window> window;
};
