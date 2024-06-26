#pragma once

#include "Camera.h"
#include "Component.h"
#include "Window.h"

class Entity;

class PlayerInput final : public Component
{
public:
    static std::shared_ptr<PlayerInput> create();

    explicit PlayerInput(AK::Badge<PlayerInput>);

    virtual void on_enabled() override;
    virtual void on_disabled() override;
    virtual void awake() override;
    virtual void update() override;

    std::weak_ptr<Entity> camera_entity;
    std::shared_ptr<Window> window;

    float player_speed = 5.0f;
    float camera_speed = 12.5f;

    std::weak_ptr<Entity> player;
    std::weak_ptr<Entity> player_model;
    std::weak_ptr<Entity> player_head;
    std::weak_ptr<Entity> camera_parent;

private:
    void process_input() const;
    void process_terminator_input() const;
    void focus_callback(i32 const focused);
    void mouse_callback(double const x, double const y);

    std::shared_ptr<Camera> m_camera;
    glm::vec3 m_camera_euler_angles_terminator = glm::vec3(-8.5f, 0.0f, 0.0f);

    glm::dvec2 m_last_mouse_position = glm::dvec2(1280.0 / 2.0, 720.0 / 2.0);
    float m_yaw = 0.0f;
    float m_pitch = 10.0f;
    bool m_mouse_just_entered = true;

    double m_sensitivity = 0.1;

    bool m_terminator_mode = false;
};
