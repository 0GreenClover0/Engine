#include "PlayerInput.h"

#include <iostream>

#include "Entity.h"
#include "Globals.h"
#include "Input.h"

std::shared_ptr<PlayerInput> PlayerInput::create()
{
    auto player_input = std::make_shared<PlayerInput>(AK::Badge<PlayerInput> {});

    return player_input;
}

PlayerInput::PlayerInput(AK::Badge<PlayerInput>)
{
}

void PlayerInput::on_enabled()
{
    // Callbacks
    Input::input->on_focused_event.attach(&PlayerInput::focus_callback, shared_from_this());
    Input::input->on_set_cursor_pos_event.attach(&PlayerInput::mouse_callback, shared_from_this());
}

void PlayerInput::on_disabled()
{
    // Callbacks
    Input::input->on_focused_event.detach(shared_from_this());
    Input::input->on_set_cursor_pos_event.detach(shared_from_this());
}

void PlayerInput::awake()
{
    if (!camera_entity.expired())
    {
        m_camera = camera_entity.lock()->get_component<Camera>();
    }
}

void PlayerInput::update()
{
    if (!camera_entity.expired() && Input::input->get_key_down(GLFW_KEY_P))
    {
        auto const camera_ent_locked = camera_entity.lock();
        m_terminator_mode = !m_terminator_mode;
        camera_ent_locked->transform->set_local_position(glm::vec3(0.0f, 2.5f, 4.0f));

        if (m_terminator_mode)
        {
            camera_ent_locked->transform->set_euler_angles(m_camera_euler_angles_terminator);
            glfwSetInputMode(window->get_glfw_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else
        {
            m_camera_euler_angles_terminator = camera_ent_locked->transform->get_euler_angles();
            glfwSetInputMode(window->get_glfw_window(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    if (!m_terminator_mode)
    {
        process_input();
    }
    else
    {
        process_terminator_input();
    }
}

void PlayerInput::process_input() const
{
    if (camera_entity.expired())
        return;

    auto const camera_locked = camera_entity.lock();

    float const current_speed = camera_speed * delta_time;

    if (!Input::input->get_key(GLFW_MOUSE_BUTTON_RIGHT))
        return;

    if (Input::input->get_key(GLFW_KEY_W))
        camera_locked->transform->set_local_position(camera_locked->transform->get_local_position() +=
                                                     current_speed * m_camera->get_front());

    if (Input::input->get_key(GLFW_KEY_S))
        camera_locked->transform->set_local_position(camera_locked->transform->get_local_position() -=
                                                     current_speed * m_camera->get_front());

    if (Input::input->get_key(GLFW_KEY_A))
        camera_locked->transform->set_local_position(camera_locked->transform->get_local_position() -=
                                                     glm::normalize(glm::cross(m_camera->get_front(), m_camera->get_up())) * current_speed);

    if (Input::input->get_key(GLFW_KEY_D))
        camera_locked->transform->set_local_position(camera_locked->transform->get_local_position() +=
                                                     glm::normalize(glm::cross(m_camera->get_front(), m_camera->get_up())) * current_speed);

    if (Input::input->get_key(GLFW_KEY_E))
        camera_locked->transform->set_local_position(camera_locked->transform->get_local_position() +=
                                                     current_speed * glm::vec3(0.0f, 1.0f, 0.0f));

    if (Input::input->get_key(GLFW_KEY_Q))
        camera_locked->transform->set_local_position(camera_locked->transform->get_local_position() -=
                                                     current_speed * glm::vec3(0.0f, 1.0f, 0.0f));
}

void PlayerInput::process_terminator_input() const
{
    if (player.expired() || camera_entity.expired() || player_model.expired() || camera_parent.expired())
        return;

    auto const player_locked = player.lock();
    auto const camera_locked = camera_entity.lock();
    auto const camera_parent_locked = camera_parent.lock();
    auto const player_model_locked = player_model.lock();

    float const current_speed = player_speed * delta_time;
    if (Input::input->get_key(GLFW_KEY_W))
        player_locked->transform->set_local_position(player_locked->transform->get_local_position() -=
                                                     current_speed * player_model_locked->transform->get_forward());

    if (Input::input->get_key(GLFW_KEY_S))
        player_locked->transform->set_local_position(player_locked->transform->get_local_position() +=
                                                     current_speed * player_model_locked->transform->get_forward());

    if (Input::input->get_key(GLFW_KEY_A))
    {
        camera_parent_locked->transform->set_euler_angles(camera_parent_locked->transform->get_euler_angles() +=
                                                          glm::vec3(0.0f, 10.0f, 0.0f) * current_speed);
        player_model_locked->transform->set_euler_angles(player_model_locked->transform->get_euler_angles() +=
                                                         glm::vec3(0.0f, 10.0f, 0.0f) * current_speed);
        camera_locked->transform->set_euler_angles(camera_locked->transform->get_euler_angles() +=
                                                   glm::vec3(0.0f, 10.0f, 0.0f) * current_speed);
    }

    if (Input::input->get_key(GLFW_KEY_D))
    {
        camera_parent_locked->transform->set_euler_angles(camera_parent_locked->transform->get_euler_angles() -=
                                                          glm::vec3(0.0f, 10.0f, 0.0f) * current_speed);
        player_model_locked->transform->set_euler_angles(player_model_locked->transform->get_euler_angles() -=
                                                         glm::vec3(0.0f, 10.0f, 0.0f) * current_speed);
        camera_locked->transform->set_euler_angles(camera_locked->transform->get_euler_angles() -=
                                                   glm::vec3(0.0f, 10.0f, 0.0f) * current_speed);
    }
}

void PlayerInput::mouse_callback(double const x, double const y)
{
    if (m_mouse_just_entered)
    {
        m_last_mouse_position.x = x;
        m_last_mouse_position.y = y;
        m_mouse_just_entered = false;
    }

    double x_offset = x - m_last_mouse_position.x;
    double y_offset = m_last_mouse_position.y - y;
    m_last_mouse_position.x = x;
    m_last_mouse_position.y = y;

    if (!Input::input->get_key(GLFW_MOUSE_BUTTON_RIGHT))
        return;

    x_offset *= m_sensitivity;
    y_offset *= m_sensitivity;

    if (m_terminator_mode)
    {
        if (player_head.expired())
            return;

        auto const head_locked = player_head.lock();
        float const current_rotation = head_locked->transform->get_euler_angles().y + y_offset;
        float const new_rotation = std::clamp(current_rotation, -40.0f, 40.0f);
        head_locked->transform->set_euler_angles(glm::vec3(0.0f, new_rotation, 0.0F));
        return;
    }

    m_yaw += x_offset;
    m_pitch = glm::clamp(m_pitch + y_offset, -89.0, 89.0);

    if (camera_entity.expired())
        return;

    camera_entity.lock()->transform->set_euler_angles(glm::vec3(m_pitch, -m_yaw, 0.0f));
}

void PlayerInput::focus_callback(i32 const focused)
{
    if (focused == 0)
        m_mouse_just_entered = true;
}
