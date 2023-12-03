#include "imgui.h"
#include "imgui_impl/imgui_impl_glfw.h"
#include "imgui_impl/imgui_impl_opengl3.h"
#include <cstdio>

#include <glad/glad.h>  // Initialize with gladLoadGL()

#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions
#include <spdlog/spdlog.h>

#define STB_IMAGE_IMPLEMENTATION
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/random.hpp>

#include "Camera.h"
#include "Cube.h"
#include "Editor.h"
#include "Entity.h"
#include "Globals.h"
#include "MainScene.h"
#include "PlanetarySystem.h"
#include "Renderer.h"
#include "Scene.h"
#include "Shader.h"
#include "stb_image.h"
#include "Window.h"

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

void process_input(GLFWwindow*);
bool is_button_pressed(GLFWwindow*, int);

bool mouse_just_entered = true;
std::shared_ptr<Window> setup_glfw();
int setup_glad();
void setup_imgui(GLFWwindow* glfw_window);

void glfw_error_callback(int, char const*);
void mouse_callback(GLFWwindow*, double, double);
void focus_callback(GLFWwindow*, int);

unsigned int generate_texture(char const* path);

float camera_movement_speed = 2.5f;

double last_frame = 0.0; // Time of last frame

std::shared_ptr<Camera> camera;

unsigned int textures_generated = 0;

glm::dvec2 last_mouse_position = glm::dvec2(static_cast<double>(screen_width) / 2.0, static_cast<double>(screen_height) / 2.0);

int main(int, char**)
{
    auto const window = setup_glfw();

    if (window == nullptr)
        return 1;

    if (setup_glad() != 0)
        return 1;

    srand(time(nullptr));

    setup_imgui(window->get_glfw_window());

    InternalMeshData::initialize();

    auto constexpr clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    glEnable(GL_DEPTH_TEST);

    std::shared_ptr<Renderer> renderer = Renderer::create();

    camera = std::make_shared<Camera>();
    Camera::set_main_camera(camera);
    camera->position = glm::vec3(0.0f, 5.0f, 10.0f);
    camera->pitch = -30.0;
    camera->update();

    auto const main_scene = std::make_shared<Scene>();
    MainScene::set_instance(main_scene);

    auto const planetary_system = Entity::create("PlanetarySystem");
    auto const planetary_system_comp = planetary_system->add_component<PlanetarySystem>();

    auto const directional_light = Entity::create("DirectionalLight");
    auto light_shader = Shader::create("./res/shaders/light.vert", "./res/shaders/light.frag");
    auto light_material = std::make_shared<Material>(light_shader);
    auto const dir_light_comp = directional_light->add_component<DirectionalLight>(DirectionalLight::create());
    dir_light_comp->diffuse = glm::vec3(1.0f, 1.0f, 0.0f);
    light_material->color = glm::vec4(dir_light_comp->diffuse.x, dir_light_comp->diffuse.y, dir_light_comp->diffuse.z, 1.0f);
    directional_light->add_component<Cube>(light_material);

    auto const container = Entity::create("Container");
    auto shader = Shader::create("./res/shaders/standard.vert", "./res/shaders/standard.frag");
    auto object_material = std::make_shared<Material>(shader);
    object_material->color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    container->add_component<Cube>("./res/textures/container.png", object_material);
    container->transform->set_local_position(glm::vec3(5.0f, 0.0f, 0.0f));
    container->transform->set_local_scale(glm::vec3(1.0f, 1.0f, 1.0f));

    // Call awake on all entities
    main_scene->awake();

    // Call start on all entities
    main_scene->start();

    bool debug_open = true;
    bool polygon_mode = false;
    float detail = 1.0f;
    float last_detail = detail;

    int nb_frames = 0;
    double frame_per_second = 0.0;
    double last_time = glfwGetTime();

    Editor::Editor const editor(main_scene);

    // Main loop
    while (!glfwWindowShouldClose(window->get_glfw_window()))
    {
        directional_light->transform->set_local_position(glm::vec3(glm::sin(glfwGetTime()) * 5.0f, 0.0f, glm::cos(glfwGetTime()) * 2.0f));
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
        ImGui::SliderFloat("Rotation X", &planetary_system_comp->x_rotation, 0.0f, 360.0f);
        ImGui::SliderFloat("Rotation Y", &planetary_system_comp->y_rotation, 0.0f, 360.0f);
        ImGui::SliderFloat("Level of detail", &detail, 0.03f, 2.0f);
        ImGui::Text("Application average %.3f ms/frame", frame_per_second);

        editor.draw_scene_save();
        ImGui::End();

        editor.draw_scene_hierarchy();

        if (!glm::epsilonEqual(last_detail, detail, 0.01f))
        {
            planetary_system_comp->change_detail(detail);
            last_detail = detail;
        }

        if (polygon_mode)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }
        else
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        planetary_system->transform->set_euler_angles(glm::vec3(planetary_system_comp->x_rotation, 0.0f, planetary_system_comp->y_rotation));

        // Input
        process_input(window->get_glfw_window());

        // Rendering
        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window->get_glfw_window(), &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update camera projection matrix
        camera->projection = glm::perspective(glm::radians(60.0f), static_cast<float>(screen_width) / static_cast<float>(screen_height), 0.1f, 100000.0f);

        main_scene->run_frame();
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

    glfwSwapInterval(1); // Enable vsync

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Capture mouse

    // Callbacks
    //glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetWindowFocusCallback(window->get_glfw_window(), focus_callback);

    return window;
}

void process_input(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float const camera_speed = camera_movement_speed * delta_time;
    if (is_button_pressed(window, GLFW_KEY_W))
        camera->position += camera_speed * camera->front;

    if (is_button_pressed(window, GLFW_KEY_S))
        camera->position -= camera_speed * camera->front;

    if (is_button_pressed(window, GLFW_KEY_A))
        camera->position -= glm::normalize(glm::cross(camera->front, camera->up)) * camera_speed;

    if (is_button_pressed(window, GLFW_KEY_D))
        camera->position += glm::normalize(glm::cross(camera->front, camera->up)) * camera_speed;
}

bool is_button_pressed(GLFWwindow *window, int const key)
{
    return glfwGetKey(window, key) == GLFW_PRESS;
}

void glfw_error_callback(int error, char const* description)
{
    (void)fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

void mouse_callback(GLFWwindow* window, double x, double y)
{
    if (mouse_just_entered)
    {
        last_mouse_position.x = x;
        last_mouse_position.y = y;
        mouse_just_entered = false;
    }

    double x_offset = x - last_mouse_position.x;
    double y_offset = last_mouse_position.y - y;
    last_mouse_position.x = x;
    last_mouse_position.y = y;

    x_offset *= camera->sensitivity;
    y_offset *= camera->sensitivity;

    camera->yaw += x_offset;
    camera->pitch += y_offset;
    camera->pitch = glm::clamp(camera->pitch, -89.0, 89.0);

    camera->update();
}

void focus_callback(GLFWwindow* window, int const focused)
{
    if (focused == 0)
        mouse_just_entered = true;
}

unsigned int generate_texture(char const* path)
{
    unsigned int texture_id;
    glGenTextures(1, &texture_id);
    glBindTexture(GL_TEXTURE_2D, texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_set_flip_vertically_on_load(true);

    int width, height, channel_count;
    unsigned char* data = stbi_load(path, &width, &height, &channel_count, 0);

    if (data == nullptr)
    {
        std::cout << "Warning. Failed to load a texutre.";
    }
    else
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    stbi_image_free(data);

    return texture_id;
}
