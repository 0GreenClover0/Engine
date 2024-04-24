#include <GLFW/glfw3.h>
#include <imgui.h>

#include "Entity.h"
#include "Input.h"
#include "LighthouseLight.h"

std::shared_ptr<LighthouseLight> LighthouseLight::create()
{
    return std::make_shared<LighthouseLight>(AK::Badge<LighthouseLight> {});
}

LighthouseLight::LighthouseLight(AK::Badge<LighthouseLight>)
{
}

void LighthouseLight::awake()
{
    set_can_tick(true);
}

void LighthouseLight::update()
{   
    glm::vec2 const position = get_position();

    entity->transform->set_local_position(glm::vec3(position.x, 0.0f, position.y));
}

void LighthouseLight::draw_editor()
{
    ImGui::InputFloat("Light range", &range);
}

glm::vec2 LighthouseLight::get_position() const
{
    float const y = Input::input->get_mouse_position().y * m_playfield_height;
    float const x = Input::input->get_mouse_position().x * (m_playfield_width - (m_playfield_additional_width * (Input::input->get_mouse_position().y + 1.0f) / 2.0f));

    return glm::vec2(x, y);
}
