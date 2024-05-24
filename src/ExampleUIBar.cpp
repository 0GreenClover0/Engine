#include "ExampleUIBar.h"

#include <GLFW/glfw3.h>
#include <imgui.h>

#include "Entity.h"
#include "ResourceManager.h"

std::shared_ptr<ExampleUIBar> ExampleUIBar::create()
{
    return std::make_shared<ExampleUIBar>();
}

void ExampleUIBar::awake()
{
    set_can_tick(true);

    auto const ui_shader = ResourceManager::get_instance().load_shader("./res/shaders/ui.hlsl", "./res/shaders/ui.hlsl");
    auto const ui_material = Material::create(ui_shader, 1);

    m_sprite_background = Entity::create("Background Sprite");
    m_sprite_background->transform->set_parent(entity->transform);
    m_sprite_value = Entity::create("Value Sprite");

    m_sprite_value->add_component<Sprite>(Sprite::create(ui_material, "./res/textures/white.jpg"));

    m_sprite_background->add_component<Sprite>(Sprite::create(ui_material, "./res/textures/black.jpg"));
    m_sprite_background->transform->set_local_position({-1.0f, -1.0f, 0.0f});
    m_sprite_background->transform->set_local_scale({0.5f, 0.1f, 1.0f});

    m_sprite_value->transform->set_parent(m_sprite_background->transform);
}

void ExampleUIBar::update()
{
    value = sin(glfwGetTime());
    m_sprite_value->transform->set_local_scale({value * 0.98f, 0.9f, 1.0f});
}

void ExampleUIBar::draw_editor()
{
    Component::draw_editor();

    ImGui::SliderFloat("Value", &value, 0.0f, 1.0f);
}
