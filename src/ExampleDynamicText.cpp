#include "ExampleDynamicText.h"

#include "Entity.h"
#include "Renderer.h"
#include "ResourceManager.h"

#include <GLFW/glfw3.h>

std::shared_ptr<ExampleDynamicText> ExampleDynamicText::create()
{
    return std::make_shared<ExampleDynamicText>();
}

void ExampleDynamicText::awake()
{
    set_can_tick(true);

    auto const ui_shader = ResourceManager::get_instance().load_shader("./res/shaders/ui.hlsl", "./res/shaders/ui.hlsl");
    auto const ui_material = Material::create(ui_shader, Renderer::ui_render_order);

    // This is example usage of ScreenText.
    m_foo = entity->add_component<ScreenText>(
        ScreenText::create(ui_material, m_example, glm::vec2(0, 0), 128, 0xff0099ff, FW1_RESTORESTATE | FW1_CENTER | FW1_VCENTER));
}

void ExampleDynamicText::update()
{
    // Rewrite the "example" array with different value every frame. ScreenText is referencing "example" so the component sees the change...
    m_example = std::to_string(sin(glfwGetTime()));

    // But you still need to update it in terms of Direct3D state. This call does that.
    m_foo->set_text(m_example);
}
