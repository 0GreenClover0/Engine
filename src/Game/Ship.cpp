#include <GLFW/glfw3.h>
#include <imgui.h>
#include <glm/gtx/vector_angle.hpp>
#include <glm/vec2.hpp>

#include "Entity.h"
#include "Input.h"
#include "Ship.h"

#include "imgui_extensions.h"
#include "Globals.h"

std::shared_ptr<Ship> Ship::create()
{
    return std::make_shared<Ship>(AK::Badge<Ship> {});
}

std::shared_ptr<Ship> Ship::create(std::shared_ptr<LighthouseLight> const& light)
{
    auto ship = std::make_shared<Ship>(AK::Badge<Ship> {});
    ship->light = light;
    
    return ship;
}

Ship::Ship(AK::Badge<Ship>)
{
}

void Ship::awake()
{
    set_can_tick(true);
}

void Ship::update()
{
    m_speed = maximum_speed;

    glm::vec2 const ship_position = { entity->transform->get_local_position().x, entity->transform->get_local_position().z };

    if (!light.expired() && light.lock()->enabled())
    {
        auto const light_locked = light.lock();
        glm::vec2 const target_position = light_locked->get_position();

        float const distance_to_light = glm::distance(ship_position, target_position);

        if (distance_to_light < light_locked->range)
        {
            follow_light(ship_position, target_position);

            m_speed = minimum_speed + ((maximum_speed + light_locked->additional_ship_speed - minimum_speed) * (distance_to_light / light_locked->range));
        }
    }

    float delta_speed = m_speed * delta_time;

    glm::vec2 speed_vector = glm::vec2(cos(glm::radians(m_direction)), sin(glm::radians(m_direction))) * delta_speed;

    entity->transform->set_local_position(entity->transform->get_local_position() + glm::vec3(speed_vector.x, 0.0f, speed_vector.y));
    entity->transform->set_euler_angles(glm::vec3(0.0f, -m_direction - 90.0f, 0.0f));
}

void Ship::draw_editor()
{
    ImGui::DragFloat("Speed", &maximum_speed, 0.001f, 0.0f, 0.5f);

    draw_ptr("Light", light);
}

void Ship::follow_light(glm::vec2 ship_position, glm::vec2 target_position)
{
    glm::vec2 const ship_direction = glm::normalize(glm::vec2(cos(glm::radians(m_direction)), sin(glm::radians(m_direction))));
    glm::vec2 const target_direction = glm::normalize(glm::vec2(target_position - ship_position));

    float const rotate_distance = glm::degrees(glm::angle(ship_direction, target_direction));

    if (rotate_distance <= visibility_range)
    {
        i32 const rotate_direction = glm::sign(ship_direction.x * target_direction.y - ship_direction.y * target_direction.x);

        m_direction += rotate_direction * turn_speed * delta_time;
    }
}
