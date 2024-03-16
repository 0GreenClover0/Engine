#pragma once

#include <glad/glad.h> // Initialize with gladLoadGL()
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions

#include <miniaudio.h>

#include <memory>

#include "Window.h"
#include "AK/Types.h"

class Engine
{
public:
    static i32 initialize();
    static void create_game();
    static void run();
    static void clean_up();

    inline static bool enable_vsync = false;
    inline static bool enable_mouse_capture = true;

    inline static ma_engine audio_engine;

    inline static std::shared_ptr<Window> window;
private:
    static i32 initialize_thirdparty();

    static i32 setup_glfw();
    static std::shared_ptr<Window> create_window();
    static void glfw_error_callback(i32 const error, char const* description);

    static i32 setup_glad();

    static void setup_imgui(GLFWwindow* glfw_window);

    static i32 setup_miniaudio();
};
