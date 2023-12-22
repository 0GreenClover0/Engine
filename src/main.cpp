#include "imgui.h"
#include "imgui_impl/imgui_impl_glfw.h"
#include "imgui_impl/imgui_impl_opengl3.h"
#include <cstdio>

#include <glad/glad.h>  // Initialize with gladLoadGL()

#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions
#include <spdlog/spdlog.h>

#define STB_IMAGE_IMPLEMENTATION
#include <iostream>

#include "Camera.h"
#include "Editor.h"
#include "Globals.h"
#include "Input.h"
#include "MainScene.h"
#include "Renderer.h"
#include "Scene.h"
#include "stb_image.h"
#include "Window.h"
#include "Game/Game.h"

#define FORCE_DEDICATED_GPU 0

#if FORCE_DEDICATED_GPU
extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

extern "C"
{
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

int screen_width  = 1280;
int screen_height = 720;

std::shared_ptr<Window> setup_glfw();
int setup_glad();
void setup_imgui(GLFWwindow* glfw_window);

void glfw_error_callback(int, char const*);

unsigned int generate_texture(char const* path);

double last_frame = 0.0; // Time of last frame

unsigned int textures_generated = 0;

int main(int, char**)
{
    auto const window = setup_glfw();

    if (window == nullptr)
        return 1;

    if (setup_glad() != 0)
        return 1;

    srand(static_cast<unsigned int>(glfwGetTime()));

    setup_imgui(window->get_glfw_window());

    auto input_system = std::make_shared<Input>(window);
    Input::set_input(input_system);

    InternalMeshData::initialize();

    auto constexpr clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    auto const renderer = Renderer::create();

    auto const main_scene = std::make_shared<Scene>();
    MainScene::set_instance(main_scene);

    auto const game = std::make_shared<Game>(window);
    game->initialize();

    // Call awake on all entities
    main_scene->awake();

    // Call start on all entities
    main_scene->start();

    renderer->initialize();

    bool debug_open = true;
    bool polygon_mode = false;

    int nb_frames = 0;
    double frame_per_second = 0.0;
    double last_time = glfwGetTime();

    Editor::Editor editor(main_scene);

    // Main loop
    while (!glfwWindowShouldClose(window->get_glfw_window()))
    {
        double const current_frame = glfwGetTime();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;

        nb_frames++;
        if (current_frame - last_time >= 1.0 )
        {
            frame_per_second = 1000.0 / static_cast<double>(nb_frames);
            nb_frames = 0;
            last_time += 1.0;
        }

        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGuiWindowFlags window_flags = 0;
        window_flags |= ImGuiWindowFlags_MenuBar;

        ImGui::Begin("Debug", &debug_open, window_flags);
        ImGui::Checkbox("Polygon mode", &polygon_mode);
        ImGui::Text("Application average %.3f ms/frame", frame_per_second);

        editor.draw_scene_save();
        ImGui::End();

        editor.draw_scene_hierarchy();
        editor.draw_inspector();

        if (polygon_mode)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        glfwGetFramebufferSize(window->get_glfw_window(), &screen_width, &screen_height);
        glViewport(0, 0, screen_width, screen_height);

        // Update camera
        if (Camera::get_main_camera() != nullptr)
        {
            Camera::get_main_camera()->set_width(static_cast<float>(screen_width));
            Camera::get_main_camera()->set_height(static_cast<float>(screen_height));
        }

        // Run frame
        main_scene->run_frame();

        // Render frame
        ImGui::Render();

        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Renderer::get_instance()->render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwMakeContextCurrent(window->get_glfw_window());
        glfwSwapBuffers(window->get_glfw_window());
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window->get_glfw_window());
    glfwTerminate();

    return 0;
}

void setup_imgui(GLFWwindow* glfw_window)
{
    // Setup Dear ImGui binding
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    // GL 4.3 + GLSL 430
    auto const glsl_version = "#version 430";
    ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Setup style
    ImGui::StyleColorsDark();
}

int setup_glad()
{
    // Initialize OpenGL loader
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
        spdlog::error("Failed to initialize OpenGL loader!");
        return 1;
    }
    spdlog::info("Successfully initialized OpenGL loader!");

    return 0;
}

std::shared_ptr<Window> setup_glfw()
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return nullptr;

    // Create window with graphics context
    auto window = std::make_shared<Window>(screen_width, screen_height);

    glfwSwapInterval(0); // Enable vsync

    glfwSetInputMode(window->get_glfw_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Capture mouse

    return window;
}

void glfw_error_callback(int error, char const* description)
{
    (void)fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}
