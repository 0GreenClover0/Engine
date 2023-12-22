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
#include <glm/gtc/random.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Camera.h"
#include "CommonEntities.h"
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

unsigned int textures_generated = 0;

glm::dvec2 last_mouse_position = glm::dvec2(static_cast<double>(screen_width) / 2.0, static_cast<double>(screen_height) / 2.0);

std::shared_ptr<Entity> camera;
std::shared_ptr<Camera> camera_comp;

float yaw = 0.0f;
float pitch = 10.0f;
constexpr double sensitivity = 0.1;

int main(int, char**)
{
    auto const window = setup_glfw();

    if (window == nullptr)
        return 1;

    if (setup_glad() != 0)
        return 1;

    srand(static_cast<unsigned int>(glfwGetTime()));

    setup_imgui(window->get_glfw_window());

    InternalMeshData::initialize();

    auto constexpr clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    auto const renderer = Renderer::create();

    auto const main_scene = std::make_shared<Scene>();
    MainScene::set_instance(main_scene);

    camera = Entity::create("Camera");
    camera->transform->set_local_position(glm::vec3(0.0f, 0.0f, 10.0f));

    camera_comp = camera->add_component<Camera>();
    Camera::set_main_camera(camera_comp);
    camera_comp->set_can_tick(true);
    camera_comp->set_fov(glm::radians(60.0f));

    auto const root = Entity::create("Root");

    auto const point_light = CommonEntities::create_point_light(glm::vec3(1.0f, 1.0f, 0.0f), root->transform);
    auto const point_light_comp = point_light->get_component<PointLight>();
    auto const point_light_material = point_light->get_component<Cube>()->material;

    auto const directional_light = CommonEntities::create_directional_light(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(-0.2f, -1.0f, 50.0f), root->transform);
    directional_light->transform->set_local_position(glm::vec3(0.0f, 5.0f, 0.0f));
    auto const directional_light_comp = directional_light->get_component<DirectionalLight>();
    auto const directional_light_material = directional_light->get_component<Cube>()->material;

    auto const directional_arrow = Entity::create("DirectionalArrow");
    directional_arrow->transform->set_parent(directional_light->transform);
    directional_arrow->transform->set_euler_angles(glm::vec3(0.0f, 90.0f, 0.0f));
    directional_arrow->add_component<Model>(Model::create("./res/models/arrow/scene.gltf", directional_light_material));

    auto const spot_light = CommonEntities::create_spot_light(glm::vec3(0.0f, 0.2f, 1.0f), root->transform);
    auto const spot_light_comp = spot_light->get_component<SpotLight>();
    auto const spot_light_material = spot_light->get_component<Cube>()->material;
    spot_light->transform->set_local_position(glm::vec3(12.0f, 0.0f, 0.0f));
    spot_light->transform->set_euler_angles(glm::vec3(-2.0f, 90.0f, -2.0f));

    auto const spot_arrow = Entity::create("SpotArrow");
    spot_arrow->transform->set_parent(spot_light->transform);
    spot_arrow->transform->set_euler_angles(glm::vec3(0.0f, 90.0f, 0.0f));
    spot_arrow->add_component<Model>(Model::create("./res/models/arrow/scene.gltf", spot_light_material));

    auto const spot_light2 = CommonEntities::create_spot_light(glm::vec3(1.0f, 0.0f, 0.0f), root->transform);
    auto const spot_light2_comp = spot_light2->get_component<SpotLight>();
    auto const spot_light2_material = spot_light2->get_component<Cube>()->material;
    spot_light2->transform->set_local_position(glm::vec3(-13.0f, 0.0f, 0.0f));
    spot_light2->transform->set_euler_angles(glm::vec3(2.0f, 0.0f, -2.0f));

    auto const spot2_arrow = Entity::create("SpotArrow");
    spot2_arrow->transform->set_parent(spot_light2->transform);
    spot2_arrow->transform->set_euler_angles(glm::vec3(0.0f, 90.0f, 0.0f));
    spot2_arrow->add_component<Model>(Model::create("./res/models/arrow/scene.gltf", spot_light2_material));

    auto instanced_shader = Shader::create("./res/shaders/standard_instanced.vert", "./res/shaders/standard.frag");
    auto standard_shader = Shader::create("./res/shaders/standard.vert", "./res/shaders/standard.frag");
    auto const cube_material = std::make_shared<Material>(instanced_shader, true);
    auto const roof_material = std::make_shared<Material>(instanced_shader, true);
    auto const floor_material = std::make_shared<Material>(standard_shader);

    auto const floor = Entity::create("Floor");
    floor->transform->set_parent(root->transform);
    floor->add_component<Cube>(Cube::create("./res/textures/stone.jpg", floor_material, true));
    floor->transform->set_local_scale(glm::vec3(1000.0f, 0.1f, 1000.0f));
    floor->transform->set_local_position(glm::vec3(0.0f, -1.0f, 0.0f));

    float house_x = -300.0f;
    float house_z = -300.0f;
    for (int32_t i = 0; i < 40000; ++i)
    {
        auto const house = CommonEntities::create_cube("House" + std::to_string(i), "./res/textures/container.png", "./res/textures/container_specular.png", cube_material);
        house->transform->set_parent(root->transform);
        house->transform->set_local_position(glm::vec3(house_x, 0.0f, house_z));
        house->transform->set_local_scale(glm::vec3(2.0f, 2.0f, 2.0f));
        house_x += 10.0f;

        auto const roof = Entity::create("Roof" + std::to_string(i));
        roof->add_component<Model>(Model::create("./res/models/pyramid3/scene.gltf", roof_material));
        roof->transform->set_parent(house->transform);
        roof->transform->set_local_position(glm::vec3(0.0f, 0.6f, 0.0f));
        roof->transform->set_local_scale(glm::vec3(0.4f, 0.4f, 0.4f));

        if (house_x >= 300.0f)
        {
            house_x = -300.0f;
            house_z += 10.0f;
        }
    }

    std::vector<std::string> const skybox_texture_paths =
    {
        "./res/textures/skybox/storforsen/posx.jpg",
        "./res/textures/skybox/storforsen/negx.jpg",
        "./res/textures/skybox/storforsen/posy.jpg",
        "./res/textures/skybox/storforsen/negy.jpg",
        "./res/textures/skybox/storforsen/posz.jpg",
        "./res/textures/skybox/storforsen/negz.jpg"
    };
    auto const skybox = CommonEntities::create_skybox(skybox_texture_paths, root->transform);

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

        float z = directional_light->transform->get_euler_angles().z;
        directional_light->transform->set_euler_angles(glm::vec3(directional_light->transform->get_euler_angles().x + delta_time * 10.0f, directional_light->transform->get_euler_angles().y, directional_light->transform->get_euler_angles().z));
        point_light->transform->set_local_position(glm::vec3(glm::sin(glfwGetTime()) * 5.0f, 0.0f, glm::cos(glfwGetTime()) * 2.0f));

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

        // Input
        process_input(window->get_glfw_window());

        // Rendering
        ImGui::Render();

        int display_w, display_h;
        glfwGetFramebufferSize(window->get_glfw_window(), &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);

        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Update camera
        camera_comp->set_width(static_cast<float>(screen_width));
        camera_comp->set_height(static_cast<float>(screen_height));

        // Run frame
        main_scene->run_frame();

        // Render frame
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
    glfwSetCursorPosCallback(window->get_glfw_window(), mouse_callback);
    glfwSetWindowFocusCallback(window->get_glfw_window(), focus_callback);

    return window;
}

void process_input(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float const camera_speed = camera_movement_speed * delta_time;
    if (is_button_pressed(window, GLFW_KEY_W))
        camera->transform->set_local_position(camera->transform->get_local_position() += camera_speed * camera_comp->get_front());

    if (is_button_pressed(window, GLFW_KEY_S))
        camera->transform->set_local_position(camera->transform->get_local_position() -= camera_speed * camera_comp->get_front());

    if (is_button_pressed(window, GLFW_KEY_A))
        camera->transform->set_local_position(camera->transform->get_local_position() -= glm::normalize(glm::cross(camera_comp->get_front(), camera_comp->get_up())) * camera_speed);

    if (is_button_pressed(window, GLFW_KEY_D))
        camera->transform->set_local_position(camera->transform->get_local_position() += glm::normalize(glm::cross(camera_comp->get_front(), camera_comp->get_up())) * camera_speed);

    if (is_button_pressed(window, GLFW_KEY_Q))
        camera->transform->set_local_position(camera->transform->get_local_position() += camera_speed * camera_comp->get_up());

    if (is_button_pressed(window, GLFW_KEY_E))
        camera->transform->set_local_position(camera->transform->get_local_position() -= camera_speed * camera_comp->get_up());
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

    x_offset *= sensitivity;
    y_offset *= sensitivity;

    yaw += x_offset;
    pitch = glm::clamp(pitch + y_offset, -89.0, 89.0);

    camera->transform->set_euler_angles(glm::vec3(pitch, -yaw, 0.0f));
}

void focus_callback(GLFWwindow* window, int const focused)
{
    if (focused == 0)
        mouse_just_entered = true;
}
