#include "Button.h"

#include "Entity.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "Sprite.h"

std::shared_ptr<Button> Button::create()
{
    auto button = std::make_shared<Button>(AK::Badge<Button> {});
    return button;
}

Button::Button(AK::Badge<Button>)
{
}

void Button::initialize()
{
    Component::initialize();

    auto const ui_shader = ResourceManager::get_instance().load_shader("./res/shaders/ui.hlsl", "./res/shaders/ui.hlsl");
    auto const ui_material = Material::create(ui_shader, Renderer::ui_render_order);
    ui_material->casts_shadows = false;

    entity->add_component(Sprite::create(ui_material, "./res/textures/white.jpg"));
}
