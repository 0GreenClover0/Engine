#pragma once

#include "Camera.h"
#include "Component.h"
#include "Window.h"

class Entity;

class PlayerInput final : public Component
{
public:
    std::shared_ptr<Entity> camera_entity;
    std::shared_ptr<Window> window;

    float player_speed = 5.0f;
    float camera_speed = 12.5f;

    void awake() override;
    void update() override;

    std::shared_ptr<Entity> player;
    std::shared_ptr<Entity> player_model;
    std::shared_ptr<Entity> player_head;
    std::shared_ptr<Entity> camera_parent;

private:
    void process_input() const;
    void process_terminator_input() const;
    void focus_callback(int const focused);
    void mouse_callback(double const x, double const y);

    std::shared_ptr<Camera> camera;
    glm::vec3 camera_euler_angles_terminator = glm::vec3(-8.5f, 0.0f, 0.0f);

    glm::dvec2 last_mouse_position = glm::dvec2(1280.0 / 2.0, 720.0 / 2.0);
    float yaw = 0.0f;
    float pitch = 10.0f;
    bool mouse_just_entered = true;

    double sensitivity = 0.1;

    bool terminator_mode = false;
};
