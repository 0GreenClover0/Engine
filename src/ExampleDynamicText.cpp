#include "ExampleDynamicText.h"
#include "Entity.h"
#include "GLFW/glfw3.h"

void ExampleDynamicText::awake()
{
    set_can_tick(true);

    // This is example usage of ScreenText.
    m_foo = entity->add_component<ScreenText>(ScreenText::create(m_example, glm::vec2(0, 0), 128, 0xff0099ff, FW1_CENTER | FW1_VCENTER));
}

void ExampleDynamicText::update()
{
    // Rewrite the "example" array with different value every frame. ScreenText is referencing "example" so the component sees the change...
    m_example = std::to_wstring(sin(glfwGetTime()));

    // But you still need to update it in terms of Direct3D state. This call does that.
    m_foo->set_text(m_example);
}
