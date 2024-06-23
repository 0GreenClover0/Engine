#include "Ship.h"

#include "AK/AK.h"
#include "Collider2D.h"
#include "Entity.h"
#include "Floater.h"
#include "Globals.h"
#include "IceBound.h"
#include "Input.h"
#include "LighthouseKeeper.h"
#include "Player.h"
#include "ShipSpawner.h"

#include <GLFW/glfw3.h>

#include <glm/gtc/random.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <glm/vec2.hpp>

#if EDITOR
#include "imgui_extensions.h"
#include <imgui.h>
#endif

std::shared_ptr<Ship> Ship::create()
{
    return std::make_shared<Ship>(AK::Badge<Ship> {});
}

std::shared_ptr<Ship> Ship::create(std::shared_ptr<LighthouseLight> const& light, std::shared_ptr<ShipSpawner> const& spawner,
                                   std::shared_ptr<ShipEyes> const& eyes)
{
    auto ship = std::make_shared<Ship>(AK::Badge<Ship> {});
    ship->light = light;
    ship->spawner = spawner;
    ship->eyes = eyes;

    return ship;
}

Ship::Ship(AK::Badge<Ship>)
{
}

void Ship::awake()
{
    set_start_direction();
    m_range_factor = ship_type_to_range_factor(type);

    set_can_tick(true);
}

void Ship::set_start_direction()
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
    m_direction += glm::linearRand(-m_start_direction_wiggle, m_start_direction_wiggle);

    update_position();
    update_rotation();
}

void Ship::update_position() const
{
    float const delta_speed = m_speed * delta_time;
    glm::vec2 const speed_vector = glm::vec2(cos(glm::radians(m_direction)), sin(glm::radians(m_direction))) * delta_speed;
    if (glm::epsilonEqual(Player::get_instance()->flash_counter, 0.0f, 0.0001f))
    {
        entity->transform->set_local_position(entity->transform->get_local_position() + glm::vec3(speed_vector.x, 0.0f, speed_vector.y));
    }
}

void Ship::update_rotation() const
{
    glm::vec3 const current_rotation = entity->transform->get_euler_angles();
    entity->transform->set_euler_angles(glm::vec3(current_rotation.x, -m_direction - 90.0f, current_rotation.z));
}

// --- State change

bool Ship::normal_state_change()
{
    behavioral_state = BehavioralState::Normal;
    return true;
}

bool Ship::pirate_state_change()
{
    if (type == ShipType::Pirates && m_pirates_in_control_counter <= 0.0f)
    {
        behavioral_state = BehavioralState::Pirate;
        return true;
    }

    return false;
}

bool Ship::control_state_change()
{
    if (!light.expired() && light.lock()->enabled())
    {
        glm::vec2 const ship_position = AK::convert_3d_to_2d(entity->transform->get_local_position());

        auto const light_locked = light.lock();
        glm::vec2 const target_position = light_locked->get_position();

        float const distance_to_light = glm::distance(ship_position, target_position);

        if (distance_to_light < Player::get_instance()->range * m_range_factor)
        {
            auto const nearest_ship = spawner.lock()->find_nearest_ship_object(light.lock()->get_position());

            if (nearest_ship.value().lock() == shared_from_this())
            {
                behavioral_state = BehavioralState::Control;
                light.lock()->controlled_ship = std::static_pointer_cast<Ship>(shared_from_this());
                my_light.lock()->diffuse = glm::vec3(1.0f, 0.8f, 1.0f);

                LevelController::get_instance()->check_tutorial_progress(TutorialProgressAction::ShipEnteredControl);

                return true;
            }
        }
    }

    return false;
}

bool Ship::avoid_state_change()
{
    if (eyes.lock()->see_obstacle)
    {
        m_avoid_direction = ((std::rand() % 2) * 2) - 1;
        behavioral_state = BehavioralState::Avoid;
        return true;
    }

    return false;
}

bool Ship::destroyed_state_change()
{
    if (is_destroyed)
    {
        behavioral_state = BehavioralState::Destroyed;

        LevelController::get_instance()->check_tutorial_progress(TutorialProgressAction::ShipDestroyed);

        if (type == ShipType::Pirates)
        {
            LevelController::get_instance()->check_tutorial_progress(TutorialProgressAction::PirateDestroyed);
        }

        return true;
    }

    return false;
}

bool Ship::in_port_state_change()
{
    if (m_is_in_port)
    {
        behavioral_state = BehavioralState::InPort;

        my_light.lock()->diffuse = glm::vec3(1.0f, 1.0f, 0.0f);
        my_light.lock()->set_pulsate(true);

        LevelController::get_instance()->check_tutorial_progress(TutorialProgressAction::ShipEnteredPort);

        return true;
    }

    return false;
}

// --- State ended

bool Ship::control_state_ended()
{
    bool result = false;

    if (!light.expired() && light.lock()->enabled())
    {
        if (light.lock()->controlled_ship.lock() != shared_from_this())
        {
            result = true;
        }

        glm::vec2 const ship_position = AK::convert_3d_to_2d(entity->transform->get_local_position());

        auto const light_locked = light.lock();
        glm::vec2 const target_position = light_locked->get_position();

        float const distance_to_light = glm::distance(ship_position, target_position);

        if (distance_to_light >= Player::get_instance()->range * m_range_factor)
        {
            result = true;
        }
    }
    else
    {
        result = true;
    }

    if (type == ShipType::Pirates && result)
    {
        my_light.lock()->diffuse = glm::vec3(1.0f, 0.0f, 0.0f);
        m_pirates_in_control_counter = Player::get_instance()->pirates_in_control;
    }

    if (result && type != ShipType::Pirates)
    {
        my_light.lock()->diffuse = glm::vec3(0.0f, 1.0f, 0.0f);
    }

    return result;
}

bool Ship::avoid_state_ended() const
{
    if (!eyes.lock()->see_obstacle)
    {
        return true;
    }

    return false;
}

// --- Behaviour

void Ship::normal_behavior()
{
    m_speed = maximum_speed;

    if (m_pirates_in_control_counter > 0.0f)
    {
        m_pirates_in_control_counter -= delta_time;
    }
}

void Ship::pirate_behavior()
{
    m_speed = maximum_speed;

    glm::vec2 const ship_position = AK::convert_3d_to_2d(entity->transform->get_local_position());

    auto const nearest_non_pirate_ship_position =
        spawner.lock()->find_nearest_non_pirate_ship(std::static_pointer_cast<Ship>(shared_from_this()));

    if (nearest_non_pirate_ship_position.has_value())
    {
        glm::vec2 const target_position = nearest_non_pirate_ship_position.value();
        follow_point(ship_position, target_position);
    }
}

void Ship::control_behavior()
{
    m_speed = maximum_speed;

    glm::vec2 const ship_position = AK::convert_3d_to_2d(entity->transform->get_local_position());

    auto const light_locked = light.lock();
    glm::vec2 const target_position = light_locked->get_position();

    float const distance_to_light = glm::distance(ship_position, target_position);

    follow_point(ship_position, target_position);
    m_speed = minimum_speed
            + ((maximum_speed + Player::get_instance()->additional_ship_speed - minimum_speed)
               * (distance_to_light / (Player::get_instance()->range * m_range_factor)));
}

void Ship::avoid_behavior()
{
    m_speed -= m_deceleration_speed * delta_time;
    m_direction += Player::get_instance()->turn_speed * m_avoid_direction * delta_time;

    if (m_speed < minimum_speed)
    {
        m_speed = minimum_speed;
    }
}

void Ship::destroyed_behavior()
{
    if (m_destroyed_counter > 0.0f)
    {
        m_destroyed_counter -= delta_time;
        entity->transform->set_local_position({entity->transform->get_local_position().x,
                                               ((m_destroyed_counter / m_destroy_time) - 1) * m_how_deep_sink_factor,
                                               entity->transform->get_local_position().z});
    }
    else
    {
        entity->destroy_immediate();
    }
}

void Ship::in_port_behavior()
{
    m_speed -= m_deceleration_speed * delta_time;

    if (m_speed < 0.0f)
        m_speed = 0.0f;
}

void Ship::stop_behavior()
{
    m_speed = 0.0f;
}

void Ship::update()
{
    if (is_out_of_room())
    {
        entity->destroy_immediate();
        return;
    }

    switch (behavioral_state)
    {
    case BehavioralState::Normal:

        if (in_port_state_change())
        {
            break;
        }
        if (destroyed_state_change())
        {
            break;
        }

        if (avoid_state_change())
        {
            break;
        }
        if (control_state_change())
        {
            break;
        }
        if (pirate_state_change())
        {
            break;
        }
        break;

    case BehavioralState::Pirate:

        if (destroyed_state_change())
        {
            break;
        }

        if (avoid_state_change())
        {
            break;
        }
        if (control_state_change())
        {
            break;
        }
        break;

    case BehavioralState::Control:

        if (in_port_state_change())
        {
            break;
        }
        if (destroyed_state_change())
        {
            break;
        }

        if (control_state_ended())
        {
            if (normal_state_change())
            {
                break;
            }
        }
        break;

    case BehavioralState::Avoid:

        if (in_port_state_change())
        {
            break;
        }
        if (destroyed_state_change())
        {
            break;
        }

        if (avoid_state_ended())
        {
            if (pirate_state_change())
            {
                break;
            }
            if (normal_state_change())
            {
                break;
            }
        }
        break;

    case BehavioralState::Destroyed:
        break;
    case BehavioralState::InPort:

        if (destroyed_state_change())
        {
            break;
        }
        break;

    case BehavioralState::Stop:

        if (control_state_change())
        {
            maximum_speed = LevelController::get_instance()->ships_speed;
            break;
        }
        if (destroyed_state_change())
        {
            break;
        }
        break;
    }

    switch (behavioral_state)
    {
    case BehavioralState::Normal:
        normal_behavior();
        update_position();
        update_rotation();
        break;

    case BehavioralState::Pirate:
        pirate_behavior();
        update_position();
        update_rotation();
        break;

    case BehavioralState::Control:
        control_behavior();
        update_position();
        update_rotation();
        break;

    case BehavioralState::Avoid:
        avoid_behavior();
        update_position();
        update_rotation();
        break;

    case BehavioralState::Destroyed:

        destroyed_behavior();
        break;

    case BehavioralState::InPort:
        in_port_behavior();
        update_position();
        update_rotation();
        break;

    case BehavioralState::Stop:
        break;
    }

    if (is_in_flash_collider)
    {
        LevelController::get_instance()->check_tutorial_progress(TutorialProgressAction::ShipInFlashCollider);
    }
}

void Ship::destroy()
{
    if (is_destroyed)
        return;

    is_destroyed = true;
    m_destroyed_counter = m_destroy_time;

    if (!floater.expired())
    {
        // Disable floater, ship is now sinking.
        floater.lock()->set_can_tick(false);
    }
}

void Ship::on_destroyed()
{
    on_ship_destroyed(static_pointer_cast<Ship>(shared_from_this()));
}

#if EDITOR
void Ship::draw_editor()
{
    Component::draw_editor();

    ImGui::DragFloat("Speed", &maximum_speed, 0.001f, 0.0f, 0.5f);

    ImGuiEx::draw_ptr("Light", light);

    if (ImGui::Button("Stop"))
    {
        behavioral_state = BehavioralState::Stop;
    }

    ImGui::Text(behaviour_state_to_string(behavioral_state).c_str());
}
#endif

void Ship::on_trigger_enter(std::shared_ptr<Collider2D> const& other)
{
    if (is_destroyed)
        return;

    if (other->entity->name == "Level_2_Flash_Collider")
    {
        is_in_flash_collider = true;
    }

    if (other->entity->get_component<Ship>() != nullptr)
    {
        destroy();
    }
    else if (other->entity->get_component<IceBound>() != nullptr && behavioral_state != BehavioralState::Stop)
    {
        destroy();
    }
    else if (!m_is_in_port && other->entity->get_component<LighthouseKeeper>() != nullptr)
    {
        destroy();
    }
}

void Ship::on_trigger_exit(std::shared_ptr<Collider2D> const& other)
{

    if (other->entity->name == "Level_2_Flash_Collider")
    {
        is_in_flash_collider = false;
    }
}

void Ship::stop()
{
    m_is_in_port = true;
}

bool Ship::is_in_port() const
{
    return m_is_in_port;
}

void Ship::set_direction(float direction)
{
    m_direction = direction;
    update_rotation();
}

void Ship::follow_point(glm::vec2 ship_position, glm::vec2 target_position)
{
    glm::vec2 const ship_direction = glm::normalize(glm::vec2(cos(glm::radians(m_direction)), sin(glm::radians(m_direction))));
    glm::vec2 const target_direction = glm::normalize(glm::vec2(target_position - ship_position));

    float const rotate_distance = glm::degrees(glm::angle(ship_direction, target_direction));

    if (rotate_distance <= m_visibility_range)
    {
        i32 const rotate_direction = glm::sign(ship_direction.x * target_direction.y - ship_direction.y * target_direction.x);

        m_direction += rotate_direction * Player::get_instance()->turn_speed * delta_time;
    }
}

bool Ship::is_out_of_room() const
{
    float const x = entity->transform->get_local_position().x;
    float const y = entity->transform->get_local_position().z;

    if (y < -LevelController::get_instance()->playfield_height + LevelController::get_instance()->playfield_y_shift - 1.0f
        || y > LevelController::get_instance()->playfield_height + LevelController::get_instance()->playfield_y_shift + 1.0f)
    {
        return true;
    }

    if (x < -(LevelController::get_instance()->playfield_width + LevelController::get_instance()->playfield_additional_width * 2.0f)
        || x > (LevelController::get_instance()->playfield_width + LevelController::get_instance()->playfield_additional_width * 2.0f))
    {
        return true;
    }

    return false;
}
