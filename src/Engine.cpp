#include "Engine.h"

#define STB_IMAGE_IMPLEMENTATION
#include <miniaudio.h>

#include "stb_image.h"

#include "Camera.h"
#include "Editor.h"
#include "Globals.h"
#include "Input.h"
#include "MainScene.h"
#include "Renderer.h"
#include "Window.h"
#include "Game/Game.h"

int32_t Engine::initialize()
{
    if (auto const result = initialize_thirdparty(); result != 0)
        return result;

    InternalMeshData::initialize();

    auto const renderer = Renderer::create();

    return 0;
}

void Engine::create_game()
{
    auto const main_scene = std::make_shared<Scene>();
    MainScene::set_instance(main_scene);

    // Custom initialization code
    auto const game = std::make_shared<Game>(window);
    game->initialize();
}

void Engine::run()
{
    Renderer::get_instance()->initialize();

    bool debug_open = true;
    bool polygon_mode = false;

    int nb_frames = 0;
    double frame_per_second = 0.0;
    double last_time = glfwGetTime();
    double last_frame = 0.0; // Time of last frame

    Editor::Editor editor(MainScene::get_instance());

    // Main loop
    while (!glfwWindowShouldClose(window->get_glfw_window()))
    {
        double const current_frame = glfwGetTime();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;

        nb_frames++;
        if (current_frame - last_time >= 1.0)
        {
            frame_per_second = 1000.0 / static_cast<double>(nb_frames);
            nb_frames = 0;
            last_time = glfwGetTime();
        }

        glfwPollEvents();
        Input::input->update_keys();

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
        MainScene::get_instance()->run_frame();

        // Render frame
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        Renderer::get_instance()->render();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwMakeContextCurrent(window->get_glfw_window());
        glfwSwapBuffers(window->get_glfw_window());
    }
}

void Engine::clean_up()
{
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window->get_glfw_window());
    glfwTerminate();
}

std::shared_ptr<Window> Engine::create_window()
{
    // Create window with graphics context
    auto new_window = std::make_shared<Window>(screen_width, screen_height, 4);

    // Enable vsync
    glfwSwapInterval(enable_vsync);

    // Capture mouse
    if (enable_mouse_capture)
        glfwSetInputMode(new_window->get_glfw_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED); 

    return new_window;
}

int32_t Engine::initialize_thirdparty()
{
    if (setup_glfw() != 0)
        return 1;

    window = create_window();

    if (window == nullptr)
        return 2;

    // NOTE: Creating input callbacks needs to happen BEFORE setting up imgui (for some unknown reason)
    auto const input_system = std::make_shared<Input>(window);
    Input::set_input(input_system);

    if (setup_glad() != 0)
        return 3;

    srand(static_cast<unsigned int>(glfwGetTime()));

    setup_imgui(window->get_glfw_window());

    if (setup_miniaudio() != 0)
        return 4;

    return 0;
}

int32_t Engine::setup_glfw()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    return 0;
}

void Engine::glfw_error_callback(int const error, char const* description)
{
    (void)fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int32_t Engine::setup_glad()
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

void Engine::setup_imgui(GLFWwindow* glfw_window)
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

int32_t Engine::setup_miniaudio()
{
    ma_engine_config config = ma_engine_config_init();
    config.channels = 2;
    config.sampleRate = 48000;
    config.listenerCount = 1;

    if (ma_engine_init(&config, &audio_engine) != MA_SUCCESS)
        return -1;

    if (ma_engine_start(&audio_engine) != MA_SUCCESS)
        return -2;

    return 0;
}
