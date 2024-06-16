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

std::shared_ptr<Button> Button::create()
{
    auto const ui_shader = ResourceManager::get_instance().load_shader("./res/shaders/ui.hlsl", "./res/shaders/ui.hlsl");
    auto const ui_material = Material::create(ui_shader, Renderer::ui_render_order);
    ui_material->casts_shadows = false;

    auto button = std::make_shared<Button>(AK::Badge<Button> {}, ui_material);

    return button;
}

Button::Button(AK::Badge<Button>, std::shared_ptr<Material> const& material) : Drawable(material)
{
}

void Button::initialize()
{
    Drawable::initialize();

    entity->add_component(Sprite::create(material, "./res/textures/white.jpg"));
    set_can_tick(true);
}

void Button::draw() const
{
}

void Button::update()
{
    calculate_corners_position();
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
