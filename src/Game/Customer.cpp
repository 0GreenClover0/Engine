#include "Customer.h"

#include "AK/AK.h"
#include "AK/Math.h"
#include "Camera.h"
#include "Collider2D.h"
#include "Entity.h"
#include "GameController.h"
#include "Globals.h"
#include "LighthouseKeeper.h"
#include "SceneSerializer.h"

#include <glm/gtc/random.hpp>

#if EDITOR
#include "imgui_extensions.h"
#endif

std::shared_ptr<Customer> Customer::create()
{
    return std::make_shared<Customer>(AK::Badge<Customer> {});
}

Customer::Customer(AK::Badge<Customer>)
{
}

void Customer::awake()
{
    set_can_tick(true);

    entity->transform->set_euler_angles(m_standing_rotation);

    m_jump_timer = glm::linearRand(m_jump_timer_min, m_jump_timer_max);
    m_spreading_arms_timer = glm::linearRand(m_spread_arms_min, m_spread_arms_max);
}

void Customer::update()
{
    if (entity == nullptr || entity->transform == nullptr || left_hand.expired() || right_hand.expired())
    {
        return;
    }

    if (GameController::get_instance()->is_moving_to_next_scene())
    {
        collider.lock()->set_enabled(false);
        return;
    }
    else
    {
        collider.lock()->set_enabled(true);
    }

    float const y = entity->transform->get_position().y;

    float const delta_time_f = static_cast<float>(delta_time);

    glm::vec3 const current_position = entity->transform->get_position();
    glm::vec2 const current_position_2d = AK::convert_3d_to_2d(current_position);
    glm::vec2 const destination_2d = AK::convert_3d_to_2d(m_destination);

    entity->transform->set_position(AK::convert_2d_to_3d(AK::move_towards(current_position_2d, destination_2d, delta_time_f), y));

    // If we are at the destination
    if (glm::epsilonEqual(current_position, m_destination, {0.3f, 0.3f, 0.3f}) == glm::bvec3(true, true, true))
    {
        // Force a jump if we are already fed
        if (m_is_fed && m_is_waiting_to_jump_to_water)
        {
            m_jump_timer = 0.0f;
            m_is_waiting_to_jump_to_water = false;
        }

        // Standing rotation
        m_desired_rotation = m_standing_rotation;
    }
    else
    {
        // Sliding rotation

        // Calculate direction vector
        glm::vec2 const direction = glm::normalize(destination_2d - current_position_2d);

        float desired_yaw = glm::degrees(std::atan2(direction.y, direction.x));

        glm::vec3 angles = entity->transform->get_euler_angles();
        m_desired_rotation = {desired_yaw, 0.0f, angles.z};

        // Keep the jump timer still, to not jump while sliding
        m_jump_timer = m_jump_timer_min;
    }

    entity->transform->set_euler_angles(AK::move_towards(entity->transform->get_euler_angles(), m_desired_rotation, 1.0f));

    m_jump_timer -= delta_time_f;

    if (m_jump_timer <= 0.0f && !m_is_jumping)
    {
        m_velocity_y = m_max_jump_velocity;

        // 10% chance to squel on jump
        if (std::rand() % 10 == 0)
        {
            auto squeal =
                Sound::play_sound_at_location("./res/audio/penguin/neutral/pneutral" + std::to_string(std::rand() % 7 + 1) + ".wav",
                                              entity->transform->get_position(), Camera::get_main_camera()->get_position());
        }

        m_is_jumping = true;

        auto const particle = SceneSerializer::load_prefab("PenguinJump");
        particle->transform->set_position(entity->transform->get_position() + glm::vec3(0.0f, 0.1f, 0.0f));
    }
    else if (m_is_jumping)
    {
        glm::vec3 new_position = entity->transform->get_position();
        new_position.y += m_velocity_y * delta_time_f;
        entity->transform->set_position(new_position);
        m_velocity_y = AK::move_towards({m_velocity_y, 0.0f}, {-3.0f, 0.0f}, 0.1f).x;

        if (m_is_fed && !m_is_waiting_to_jump_to_water)
        {
            if (entity->transform->get_position().y < 0.0f && !m_has_splashed)
            {
                auto splash =
                    Sound::play_sound_at_location("./res/audio/penguin/jump/wodnyskok" + std::to_string(std::rand() % 4 + 1) + ".wav",
                                                  entity->transform->get_position(), Camera::get_main_camera()->get_position());
                splash->set_volume(8.0f);
                m_has_splashed = true;
            }
            if (entity->transform->get_position().y <= desired_height - 5.0f)
            {
                entity->destroy_immediate();
                return;
            }
        }
        else if (entity->transform->get_position().y <= desired_height)
        {
            m_jump_timer = glm::linearRand(m_jump_timer_min, m_jump_timer_max);
            m_is_jumping = false;
            m_velocity_y = 0.0f;
        }
    }

    m_spreading_arms_timer -= delta_time_f;
    if (m_spreading_arms_timer <= 0.0f && !m_is_spreading_arms && !m_is_unspreading_arms)
    {
        m_is_spreading_arms = true;
    }
    else if (m_is_spreading_arms || m_is_unspreading_arms)
    {
        auto const left = left_hand.lock();
        auto const right = right_hand.lock();

        glm::vec3 left_euler = left->transform->get_euler_angles();
        float left_rotation = left_euler.x;

        glm::vec3 right_euler = right->transform->get_euler_angles();
        float right_rotation = right_euler.x;

        if (m_is_spreading_arms)
        {
            left_rotation += delta_time_f * m_spreading_arms_speed;
            right_rotation -= delta_time_f * m_spreading_arms_speed;

            if (left_rotation > m_max_left_arm_spread)
            {
                m_is_spreading_arms = false;
                m_is_unspreading_arms = true;

                left_rotation = m_max_left_arm_spread;
                right_rotation = -m_max_left_arm_spread;
            }
        }
        else
        {
            left_rotation -= delta_time_f * m_spreading_arms_speed;
            right_rotation += delta_time_f * m_spreading_arms_speed;

            if (left_rotation < 0.0f)
            {
                m_is_unspreading_arms = false;

                left_rotation = 0.0f;
                right_rotation = 0.0f;

                m_spreading_arms_timer = glm::linearRand(m_spread_arms_min, m_spread_arms_max);
            }
        }

        left_euler.x = left_rotation;
        right_euler.x = right_rotation;

        left->transform->set_euler_angles(left_euler);
        right->transform->set_euler_angles(right_euler);
    }
}

void Customer::on_collision_enter(std::shared_ptr<Collider2D> const& other)
{
    auto const keeper = other->entity->get_component<LighthouseKeeper>();

    if (keeper == nullptr)
    {
        return;
    }

    auto const collider_locked = collider.lock();
    collider_locked->add_force(keeper->get_speed() * 0.05f);
    collider_locked->velocity = glm::clamp(collider_locked->velocity, {-3.0f, -3.0f}, {3.0f, 3.0f});
}

#if EDITOR
void Customer::draw_editor()
{
    Component::draw_editor();

    ImGuiEx::draw_ptr("Collider", collider);
    ImGuiEx::draw_ptr("Left Hand", left_hand);
    ImGuiEx::draw_ptr("Right Hand", right_hand);
}
#endif

void Customer::feed(glm::vec3 const& destination)
{
    set_destination(destination);
    auto squeal = Sound::play_sound_at_location("./res/audio/penguin/happy/phappy" + std::to_string(std::rand() % 6 + 1) + ".wav",
                                                entity->transform->get_position(), Camera::get_main_camera()->get_position());
    m_is_fed = true;
    m_is_waiting_to_jump_to_water = true;
}

void Customer::set_destination(glm::vec3 const& destination)
{
    m_destination = destination;
    m_destination.y = desired_height;
}
