#include "ExampleUIBar.h"
#include "Entity.h"
#include "GLFW/glfw3.h"
#include "ShaderFactory.h"

std::shared_ptr<ExampleUIBar> ExampleUIBar::create()
{
    return std::make_shared<ExampleUIBar>();
}

void ExampleUIBar::awake()
{
    set_can_tick(true);

    auto const ui_shader = ShaderFactory::create("./res/shaders/ui.hlsl", "./res/shaders/ui.hlsl");
    auto const ui_material = Material::create(ui_shader);

    m_sprite_background = Entity::create("Background Sprite");
    m_sprite_value = Entity::create("Value Sprite");

    m_sprite_value->add_component<Sprite>(Sprite::create(ui_material, "./res/textures/white.jpg"));
    m_sprite_value->transform->set_local_position({ -1.0f, -1.0f, 0.0f });
    m_sprite_value->transform->set_local_scale({ 0.4f, 0.08f, 1.0f });

    m_sprite_background->add_component<Sprite>(Sprite::create(ui_material, "./res/textures/black.jpg"));
    m_sprite_background->transform->set_local_position({ -1.0f, -1.0f, 0.0f });
    m_sprite_background->transform->set_local_scale({ 0.5f, 0.1f, 1.0f });
}

void ExampleUIBar::update()
{
    m_value = sin(glfwGetTime());
    m_sprite_value->transform->set_local_scale({ 0.49f * m_value, 0.08f, 1.0f });
}
