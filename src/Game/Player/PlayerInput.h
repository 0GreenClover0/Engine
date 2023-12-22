#pragma once

#include "Camera.h"
#include "Component.h"
#include "Window.h"

class PlayerInput final : public Component
{
public:
    std::shared_ptr<Entity> camera_entity;
    std::shared_ptr<Window> window;

    float camera_speed = 2.5f;

    void awake() override;
    void update() override;
private:
    void process_input() const;
    void focus_callback(int const focused);
    void mouse_callback(double const x, double const y);

    std::shared_ptr<Camera> camera;

    glm::dvec2 last_mouse_position = glm::dvec2(1280.0 / 2.0, 720.0 / 2.0);
    float yaw = 0.0f;
    float pitch = 10.0f;
    bool mouse_just_entered = true;

    double sensitivity = 0.1;
};
