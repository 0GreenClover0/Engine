#include "ExampleUIBar.h"

#include <GLFW/glfw3.h>
#include <imgui.h>

#include "Entity.h"
#include "LighthouseKeeper.h"
#include "Globals.h"

#include "Lighthouse.h"
#include "AK/AK.h"
#include "Factory.h"
#include "LevelController.h"
#include "Port.h"

#include <imgui_extensions.h>

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

    handle_input();
}

void LighthouseKeeper::draw_editor()
{
    ImGuiEx::InputFloat("Acceleration", &acceleration);
    ImGuiEx::InputFloat("Deceleration", &deceleration);
    ImGuiEx::InputFloat("Maximum speed", &maximum_speed);

    ImGuiEx::draw_ptr("Lighthouse", lighthouse);
}

void LighthouseKeeper::handle_input() const
{
    auto const& factories = LevelController::get_instance()->factories;
    if (factories.size() > 0 && Input::input->get_key_down(GLFW_KEY_SPACE))
    {
        std::shared_ptr<Factory> closest_factory = factories[0].lock();
        float closest_distance = distance(
            AK::convert_3d_to_2d(closest_factory->entity->transform->get_position()),
            AK::convert_3d_to_2d(entity->transform->get_position())
        );

        for (u32 i = 1; i < factories.size(); ++i)
        {
            auto const factory_locked = factories[i].lock();
            float const distance = glm::distance(
                AK::convert_3d_to_2d(factory_locked->entity->transform->get_position()),
                AK::convert_3d_to_2d(entity->transform->get_position())
            );

            if (distance < closest_distance)
            {
                closest_distance = distance;
                closest_factory = factory_locked;
            }
        }

        if (closest_distance < interact_with_factory_distance)
        {
            if (closest_factory->interact())
            {
                return;
            }
        }
    }

    if (!port.expired() && Input::input->get_key_down(GLFW_KEY_SPACE))
    {
        auto const port_locked = port.lock();
        auto const port_transform = port_locked->entity->transform;

        glm::vec2 const keeper_position = AK::convert_3d_to_2d(entity->transform->get_position());
        glm::vec2 const port_position = AK::convert_3d_to_2d(port_transform->get_position());

        if (distance(keeper_position, port_position) < port_locked->get_interactable_distance())
        {
            if (port_locked->interact())
            {
                return;
            }
        }
    }

    if (!lighthouse.expired() && Input::input->get_key_down(GLFW_KEY_SPACE))
    {
        auto const lighthouse_locked = lighthouse.lock();
        auto const lighthouse_transform = lighthouse_locked->entity->transform;

        glm::vec2 const keeper_position = AK::convert_3d_to_2d(entity->transform->get_position());
        glm::vec2 const lighthouse_position = AK::convert_3d_to_2d(lighthouse_transform->get_position());

        if (distance(keeper_position, lighthouse_position) < lighthouse_locked->enterable_distance)
        {
            lighthouse_locked->enter();
            entity->destroy_immediate();
            return;
        }
    }
}
