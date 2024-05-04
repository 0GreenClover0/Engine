#include "ExampleUIBar.h"

#include <GLFW/glfw3.h>
#include <imgui.h>

#include "Entity.h"
#include "LighthouseKeeper.h"
#include "Globals.h"

std::shared_ptr<LighthouseKeeper> LighthouseKeeper::create()
{
    return std::make_shared<LighthouseKeeper>(AK::Badge<LighthouseKeeper> {});
}

LighthouseKeeper::LighthouseKeeper(AK::Badge<LighthouseKeeper>)
{
}

void LighthouseKeeper::awake()
{
    set_can_tick(true);
}

void LighthouseKeeper::update()
{
    i32 horizontal = 0;
    i32 vertical = 0;

    if (Input::input->get_key(GLFW_KEY_D))
    {
        horizontal++;
    }
    if (Input::input->get_key(GLFW_KEY_A))
    {
        horizontal--;
    }
    if (Input::input->get_key(GLFW_KEY_W))
    {
        vertical--;
    }
    if (Input::input->get_key(GLFW_KEY_S))
    {
        vertical++;
    }

    m_speed.x += horizontal * acceleration;
    m_speed.y += vertical * acceleration;

    if (horizontal == 0)
    {
        m_speed.x -= glm::sign(m_speed.x) * deceleration;
    }

    if (vertical == 0)
    {
        m_speed.y -= glm::sign(m_speed.y) * deceleration;
    }

    if (abs(m_speed.x) < deceleration)
    {
        m_speed.x = 0.0f;
    }

    if (abs(m_speed.y) < deceleration)
    {
        m_speed.y = 0.0f;
    }

    if (glm::length(m_speed) > maximum_speed)
    {
        m_speed = glm::normalize(m_speed) * maximum_speed;
    }

    glm::vec3 speed_vector = glm::vec3(m_speed.x, 0.0f, m_speed.y);
    speed_vector *= delta_time;

    entity->transform->set_local_position(entity->transform->get_local_position() + speed_vector);
}

void LighthouseKeeper::draw_editor()
{
    ImGui::InputFloat("Acceleration", &acceleration);
    ImGui::InputFloat("Deceleration", &deceleration);
    ImGui::InputFloat("Maximum speed", &maximum_speed);
}
