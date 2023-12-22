#include "PlayerInput.h"

#include <iostream>

#include "Entity.h"
#include "Globals.h"
#include "Input.h"

void PlayerInput::awake()
{
    camera = camera_entity->get_component<Camera>();

    // Callbacks
    Input::input->focus_callback_impl = [this](int const focused){
        this->focus_callback(focused);
    };

    Input::input->mouse_callback_impl = [this](double const x, double const y) {
        this->mouse_callback(x, y);
    };
}

void PlayerInput::update()
{
    process_input();
}

void PlayerInput::process_input() const
{
    float const current_speed = camera_speed * delta_time;
    if (Input::input->is_key_down(GLFW_KEY_W))
        camera_entity->transform->set_local_position(camera_entity->transform->get_local_position() += current_speed * camera->get_front());

    if (Input::input->is_key_down(GLFW_KEY_S))
        camera_entity->transform->set_local_position(camera_entity->transform->get_local_position() -= current_speed * camera->get_front());

    if (Input::input->is_key_down(GLFW_KEY_A))
        camera_entity->transform->set_local_position(camera_entity->transform->get_local_position() -= glm::normalize(glm::cross(camera->get_front(), camera->get_up())) * current_speed);

    if (Input::input->is_key_down(GLFW_KEY_D))
        camera_entity->transform->set_local_position(camera_entity->transform->get_local_position() += glm::normalize(glm::cross(camera->get_front(), camera->get_up())) * current_speed);

    if (Input::input->is_key_down(GLFW_KEY_Q))
        camera_entity->transform->set_local_position(camera_entity->transform->get_local_position() += current_speed * camera->get_up());

    if (Input::input->is_key_down(GLFW_KEY_E))
        camera_entity->transform->set_local_position(camera_entity->transform->get_local_position() -= current_speed * camera->get_up());
}

void PlayerInput::mouse_callback(double const x, double const y)
{
    if (mouse_just_entered)
    {
        last_mouse_position.x = x;
        last_mouse_position.y = y;
        mouse_just_entered = false;
    }

    double x_offset = x - last_mouse_position.x;
    double y_offset = last_mouse_position.y - y;
    last_mouse_position.x = x;
    last_mouse_position.y = y;

    x_offset *= sensitivity;
    y_offset *= sensitivity;

    yaw += x_offset;
    pitch = glm::clamp(pitch + y_offset, -89.0, 89.0);

    camera_entity->transform->set_euler_angles(glm::vec3(pitch, -yaw, 0.0f));
}

void PlayerInput::focus_callback(int const focused)
{
    if (focused == 0)
        mouse_just_entered = true;
}
