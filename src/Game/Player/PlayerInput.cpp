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

void PlayerInput::awake()
{
    m_camera = camera_entity->get_component<Camera>();

    // Callbacks
    Input::input->on_focused_event.attach(&PlayerInput::focus_callback, shared_from_this());
    Input::input->on_set_cursor_pos_event.attach(&PlayerInput::mouse_callback, shared_from_this());
}

void PlayerInput::update()
{
    if (Input::input->get_key_down(GLFW_KEY_P))
    {
        m_terminator_mode = !m_terminator_mode;
        camera_entity->transform->set_local_position(glm::vec3(0.0f, 2.5f, 4.0f));

        if (m_terminator_mode)
        {
            camera_entity->transform->set_euler_angles(m_camera_euler_angles_terminator);
            glfwSetInputMode(window->get_glfw_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
        else
        {
            m_camera_euler_angles_terminator = camera_entity->transform->get_euler_angles();
            glfwSetInputMode(window->get_glfw_window(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }
    if (Input::input->get_key_down(GLFW_KEY_SPACE))
    {
        if (glfwGetInputMode(window->get_glfw_window(), GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
        {
            glfwSetInputMode(window->get_glfw_window(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
        else
        {
            glfwSetInputMode(window->get_glfw_window(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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
    float const current_speed = camera_speed * delta_time;

    if (!Input::input->get_key(GLFW_MOUSE_BUTTON_RIGHT))
        return;

    if (Input::input->get_key(GLFW_KEY_W))
        camera_entity->transform->set_local_position(camera_entity->transform->get_local_position() += current_speed * m_camera->get_front());

    if (Input::input->get_key(GLFW_KEY_S))
        camera_entity->transform->set_local_position(camera_entity->transform->get_local_position() -= current_speed * m_camera->get_front());

    if (Input::input->get_key(GLFW_KEY_A))
        camera_entity->transform->set_local_position(camera_entity->transform->get_local_position() -= glm::normalize(glm::cross(m_camera->get_front(), m_camera->get_up())) * current_speed);

    if (Input::input->get_key(GLFW_KEY_D))
        camera_entity->transform->set_local_position(camera_entity->transform->get_local_position() += glm::normalize(glm::cross(m_camera->get_front(), m_camera->get_up())) * current_speed);

    if (Input::input->get_key(GLFW_KEY_Q))
        camera_entity->transform->set_local_position(camera_entity->transform->get_local_position() += current_speed * glm::vec3(0.0f, 1.0f, 0.0f));

    if (Input::input->get_key(GLFW_KEY_E))
        camera_entity->transform->set_local_position(camera_entity->transform->get_local_position() -= current_speed * glm::vec3(0.0f, 1.0f, 0.0f));
}

void PlayerInput::process_terminator_input() const
{
    float const current_speed = player_speed * delta_time;
    if (Input::input->get_key(GLFW_KEY_W))
        player->transform->set_local_position(player->transform->get_local_position() -= current_speed * player_model->transform->get_forward());

    if (Input::input->get_key(GLFW_KEY_S))
        player->transform->set_local_position(player->transform->get_local_position() += current_speed * player_model->transform->get_forward());

    if (Input::input->get_key(GLFW_KEY_A))
    {
        camera_parent->transform->set_euler_angles(camera_parent->transform->get_euler_angles() += glm::vec3(0.0f, 10.0f, 0.0f) * current_speed);
        player_model->transform->set_euler_angles(player_model->transform->get_euler_angles() += glm::vec3(0.0f, 10.0f, 0.0f) * current_speed);
        camera_entity->transform->set_euler_angles(camera_entity->transform->get_euler_angles() += glm::vec3(0.0f, 10.0f, 0.0f) * current_speed);
    }

    if (Input::input->get_key(GLFW_KEY_D))
    {
        camera_parent->transform->set_euler_angles(camera_parent->transform->get_euler_angles() -= glm::vec3(0.0f, 10.0f, 0.0f) * current_speed);
        player_model->transform->set_euler_angles(player_model->transform->get_euler_angles() -= glm::vec3(0.0f, 10.0f, 0.0f) * current_speed);
        camera_entity->transform->set_euler_angles(camera_entity->transform->get_euler_angles() -= glm::vec3(0.0f, 10.0f, 0.0f) * current_speed);
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
        float const current_rotation = player_head->transform->get_euler_angles().y + y_offset;
        float const new_rotation = std::clamp(current_rotation, -40.0f, 40.0f);
        player_head->transform->set_euler_angles(glm::vec3(0.0f, new_rotation, 0.0F));
        return;
    }

    m_yaw += x_offset;
    m_pitch = glm::clamp(m_pitch + y_offset, -89.0, 89.0);

    camera_entity->transform->set_euler_angles(glm::vec3(m_pitch, -m_yaw, 0.0f));
}

void PlayerInput::focus_callback(i32 const focused)
{
    if (focused == 0)
        m_mouse_just_entered = true;
}
