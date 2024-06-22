#include "Button.h"

#include "AK/AK.h"
#include "AK/Math.h"
#include "Editor.h"
#include "Entity.h"
#include "Input.h"
#include "Renderer.h"
#include "RendererDX11.h"
#include "ResourceManager.h"
#include "Sprite.h"

#if EDITOR
#include "imgui_stdlib.h"
#endif

std::shared_ptr<Button> Button::create()
{
    auto const ui_shader = ResourceManager::get_instance().load_shader("./res/shaders/ui.hlsl", "./res/shaders/ui.hlsl");
    auto const ui_material = Material::create(ui_shader, Renderer::ui_render_order + 1);
    ui_material->casts_shadows = false;

    auto button = std::make_shared<Button>(AK::Badge<Button> {}, ui_material);

    button->prepare();

    return button;
}

Button::Button(AK::Badge<Button>, std::shared_ptr<Material> const& material) : Drawable(material)
{
}

void Button::initialize()
{
    Drawable::initialize();
    set_can_tick(true);
    m_path = path_default;
}

void Button::draw() const
{
    if (m_rasterizer_draw_type == RasterizerDrawType::None)
    {
        return;
    }

    // Either wireframe or solid for individual model
    Renderer::get_instance()->set_rasterizer_draw_type(m_rasterizer_draw_type);

    if (m_mesh != nullptr)
    {
        m_mesh->draw();
    }

    Renderer::get_instance()->restore_default_rasterizer_draw_type();
}

#if EDITOR
void Button::draw_editor()
{
    Drawable::draw_editor();

    ImGui::InputText("Default Image Path", &path_default);

    // Just for instant feedback in editor mode. Hover and click don't work in editor anyway.
    if (ImGui::IsItemDeactivatedAfterEdit())
    {
        m_path = path_default;
        reprepare();
    }

    ImGui::InputText("Hovered Image Path", &path_hovered);
    ImGui::InputText("Pressed Image Path", &path_pressed);

    ImGui::Separator();

    ImGui::Text("Size Calculator");

    static float width = 1;
    static float height = 1;
    static float result = 1;
    static float img_scale = 1;

    bool is_size_changed = false;

    is_size_changed |= ImGui::InputFloat("Width: ", &width);
    is_size_changed |= ImGui::InputFloat("Height: ", &height);

    if (is_size_changed)
    {
        float x = (9 * width) / 16;
        result = x / height;
    }

    ImGui::Text(("Result: " + std::to_string(result)).c_str());

    if (ImGui::Button("Applay to width"))
    {
        glm::vec3 scale = entity->transform->get_local_scale();
        entity->transform->set_local_scale({scale.x * result, scale.y, 1.0f});
    }

    ImGui::SameLine();

    if (ImGui::Button("Applay to height"))
    {
        glm::vec3 scale = entity->transform->get_local_scale();
        entity->transform->set_local_scale({scale.x, scale.y * result, 1.0f});
    }

    ImGui::InputFloat("Scale: ", &img_scale);

    if (ImGui::Button("Applay scale"))
    {
        glm::vec3 scale = entity->transform->get_local_scale();
        entity->transform->set_local_scale({scale.x * img_scale, scale.y * img_scale, 1.0f});
    }
}
#endif

void Button::reprepare()
{
    prepare();
}

void Button::update()
{
    calculate_corners_position();
    perform_hover_and_click_checks();
}

void Button::perform_hover_and_click_checks()
{
    if (is_hovered() && !m_previous_frame_hovered)
    {
        on_hovered();
        m_previous_frame_hovered = true;
        m_path = path_hovered;
        reprepare();
    }

    if (!is_hovered() && m_previous_frame_hovered)
    {
        on_unhovered();
        m_previous_frame_hovered = false;
        m_path = path_default;
        reprepare();
    }

    if (is_pressed() && !m_previous_frame_clicked)
    {
        on_clicked();
        m_previous_frame_clicked = true;
        m_path = path_pressed;
        reprepare();
    }

    if (!is_pressed() && m_previous_frame_clicked)
    {
        on_unclicked();
        m_previous_frame_clicked = false;
        m_path = is_hovered() ? path_hovered : path_default;
        reprepare();
    }
}

bool Button::is_hovered() const
{
    glm::vec2 screen_size = {};
    screen_size.x = RendererDX11::get_instance_dx11()->screen_width;
    screen_size.y = RendererDX11::get_instance_dx11()->screen_height;

#if EDITOR
    glm::vec2 const game_pos = Editor::Editor::get_instance()->get_game_position();
    glm::vec2 const game_size = Editor::Editor::get_instance()->get_game_size();
#endif

    glm::vec2 mouse_pos_screen_space = {};
    glm::vec2 mouse_pos_pixels = {};

    std::array const corners = {top_left_corner, top_right_corner, bottom_left_corner, bottom_right_corner};

    mouse_pos_screen_space = Input::input->get_mouse_position();
    mouse_pos_pixels = {AK::Math::map_range_clamped(-1.0f, 1.0f, 0.0f, screen_size.x, mouse_pos_screen_space.x),
                        AK::Math::map_range_clamped(-1.0f, 1.0f, 0.0f, screen_size.y, mouse_pos_screen_space.y)};

#if EDITOR
    if (Editor::Editor::get_instance()->is_rendering_to_editor())
    {
        glm::vec2 const editor_window_corners[4] = {glm::vec2(game_pos.x, game_pos.y), glm::vec2(game_pos.x + game_size.x, game_pos.y),
                                                    glm::vec2(game_pos.x, game_pos.y + game_size.y),
                                                    glm::vec2(game_pos.x + game_size.x, game_pos.y + game_size.y)};

        mouse_pos_pixels = {
            AK::Math::map_range_clamped(editor_window_corners[0].x, editor_window_corners[1].x, 0.0f, screen_size.x, mouse_pos_pixels.x)
                - 16.0f,
            AK::Math::map_range_clamped(editor_window_corners[0].y, editor_window_corners[2].y, 0.0f, screen_size.y, mouse_pos_pixels.y)
                - 64.0f};
    }
#endif

    return AK::Math::is_point_inside_rectangle(mouse_pos_pixels, corners);
}

bool Button::is_pressed() const
{
    return (Input::input->get_key(GLFW_MOUSE_BUTTON_LEFT) || Input::input->get_key(GLFW_MOUSE_BUTTON_RIGHT)) && is_hovered();
}

void Button::calculate_corners_position()
{
    glm::vec2 screen_size = {};
    screen_size.x = RendererDX11::get_instance_dx11()->screen_width;
    screen_size.y = RendererDX11::get_instance_dx11()->screen_height;

    glm::vec2 const world_top_left = {-1.0f * entity->transform->get_scale().x + entity->transform->get_position().x,
                                      -1.0f * entity->transform->get_scale().y - entity->transform->get_position().y};

    top_left_corner = {AK::Math::map_range_clamped(-1.0f, 1.0f, 0.0f, screen_size.x, world_top_left.x),
                       AK::Math::map_range_clamped(-1.0f, 1.0f, 0.0f, screen_size.y, world_top_left.y)};

    glm::vec2 const world_top_right = {1.0f * entity->transform->get_scale().x + entity->transform->get_position().x,
                                       -1.0f * entity->transform->get_scale().y - entity->transform->get_position().y};

    top_right_corner = {AK::Math::map_range_clamped(-1.0f, 1.0f, 0.0f, screen_size.x, world_top_right.x),
                        AK::Math::map_range_clamped(-1.0f, 1.0f, 0.0f, screen_size.y, world_top_right.y)};

    glm::vec2 const world_bottom_left = {-1.0f * entity->transform->get_scale().x + entity->transform->get_position().x,
                                         1.0f * entity->transform->get_scale().y - entity->transform->get_position().y};

    bottom_left_corner = {AK::Math::map_range_clamped(-1.0f, 1.0f, 0.0f, screen_size.x, world_bottom_left.x),
                          AK::Math::map_range_clamped(-1.0f, 1.0f, 0.0f, screen_size.y, world_bottom_left.y)};

    glm::vec2 const world_bottom_right = {1.0f * entity->transform->get_scale().x + entity->transform->get_position().x,
                                          1.0f * entity->transform->get_scale().y - entity->transform->get_position().y};

    bottom_right_corner = {AK::Math::map_range_clamped(-1.0f, 1.0f, 0.0f, screen_size.x, world_bottom_right.x),
                           AK::Math::map_range_clamped(-1.0f, 1.0f, 0.0f, screen_size.y, world_bottom_right.y)};
}

void Button::prepare()
{
    m_mesh = create_sprite();
}

std::shared_ptr<Mesh> Button::create_sprite() const
{
    std::vector<Vertex> const vertices = {
        {glm::vec3(-1.0f, -1.0f, 0.0f), {}, {0.0f, 0.0f}}, // bottom left
        {glm::vec3(1.0f, -1.0f, 0.0f), {}, {1.0f, 0.0f}}, // bottom right
        {glm::vec3(1.0f, 1.0f, 0.0f), {}, {1.0f, 1.0f}}, // top right
        {glm::vec3(-1.0f, 1.0f, 0.0f), {}, {0.0f, 1.0f}}, // top left
    };

    std::vector<u32> const indices = {0, 1, 2, 0, 2, 3};

    std::vector<std::shared_ptr<Texture>> textures;

    std::vector<std::shared_ptr<Texture>> diffuse_maps = {};
    TextureSettings texture_settings = {};
    texture_settings.wrap_mode_x = TextureWrapMode::ClampToEdge;
    texture_settings.wrap_mode_y = TextureWrapMode::ClampToEdge;

    if (!m_path.empty())
        diffuse_maps.emplace_back(ResourceManager::get_instance().load_texture(m_path, TextureType::Diffuse, texture_settings));

    textures.insert(textures.end(), diffuse_maps.begin(), diffuse_maps.end());

    return ResourceManager::get_instance().load_mesh(0, m_path, vertices, indices, textures, DrawType::Triangles, material);
}
