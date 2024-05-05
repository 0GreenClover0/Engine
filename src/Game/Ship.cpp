#include <GLFW/glfw3.h>
#include <imgui.h>
#include <glm/gtx/vector_angle.hpp>
#include <glm/vec2.hpp>
#include <glm/gtc/random.hpp>

#include "Entity.h"
#include "Input.h"
#include "Ship.h"

#include "imgui_extensions.h"
#include "Globals.h"
#include "AK/AK.h"

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
    glm::vec2 const ship_position = AK::convert_3d_to_2d(entity->transform->get_local_position());

    if (glm::epsilonEqual(ship_position, {0.0f, 0.0f}, 0.0001f) == glm::bvec2(true, true))
    {
        set_can_tick(true);
        return;
    }
    glm::vec2 const target_direction = glm::normalize(glm::vec2(glm::vec2(0.0f, 0.0f) - ship_position));
    i32 const rotate_direction = glm::sign(1.0f * target_direction.y - 0.0f * target_direction.x);
    m_direction = glm::degrees(glm::angle(glm::vec2(1.0f, 0.0f), target_direction)) * rotate_direction;
    m_direction += glm::linearRand(-start_direction_wiggle, start_direction_wiggle);

    set_can_tick(true);
}

void Ship::update()
{
    if (is_out_of_room())
    {
        entity->destroy_immediate();
        return;
    }

    //TODO Add correct conditions for destroying ships
    if (Input::input->get_key_down(GLFW_KEY_F3))
    {
        destroy();
    }

    if (is_destroyed)
    {
        if (destroyed_counter > 0.0f)
        {
            destroyed_counter -= delta_time;
            entity->transform->set_local_position({
                entity->transform->get_local_position().x,
                ((destroyed_counter / destroy_time) - 1) * m_how_deep_sink_factor,
                entity->transform->get_local_position().z });
        }
        else
        {
            entity->destroy_immediate();
        }

        return;
    }

    m_speed = maximum_speed;

    glm::vec2 const ship_position = AK::convert_3d_to_2d(entity->transform->get_local_position());

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

    if (glm::epsilonEqual(GameController::get_instance()->flash_counter, 0.0f, 0.0001f))
    {
        entity->transform->set_local_position(entity->transform->get_local_position() + glm::vec3(speed_vector.x, 0.0f, speed_vector.y));
    }
    entity->transform->set_euler_angles(glm::vec3(0.0f, -m_direction - 90.0f, 0.0f));
}

void Ship::destroy()
{
    is_destroyed = true;
    destroyed_counter = destroy_time;
}

void Ship::on_destroyed()
{
    on_ship_destroyed(static_pointer_cast<Ship>(shared_from_this()));
}

void Ship::draw_editor()
{
    ImGui::DragFloat("Speed", &maximum_speed, 0.001f, 0.0f, 0.5f);

    ImGuiEx::draw_ptr("Light", light);
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

bool Ship::is_out_of_room() const
{
    float const x = entity->transform->get_local_position().x;
    float const y = entity->transform->get_local_position().z;

    if (y < -GameController::get_instance()->playfield_height || y > GameController::get_instance()->playfield_height)
    {
        return true;
    }

    if (x < -(GameController::get_instance()->playfield_width + GameController::get_instance()->playfield_additional_width) || x > (GameController::get_instance()->playfield_width + GameController::get_instance()->playfield_additional_width))
    {
        return true;
    }

    return false;
}
