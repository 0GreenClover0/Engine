#pragma once

#include "Component.h"
#include "LighthouseLight.h"

class Ship final : public Component
{
public:
    static std::shared_ptr<Ship> create();
    static std::shared_ptr<Ship> create(std::shared_ptr<LighthouseLight> const& light);

    explicit Ship(AK::Badge<Ship>);

    virtual void awake() override;
    virtual void update() override;
    virtual void on_destroyed() override;
    virtual void draw_editor() override;

    float minimum_speed = 0.11f;
    float maximum_speed = 0.23f;

    float turn_speed = 15.0f;
    i32 visibility_range = 110;
    float start_direction_wiggle = 15.0f;

    std::weak_ptr<LighthouseLight> light = {};

    Event<void(std::shared_ptr<Ship>)> on_ship_destroyed;

private:
    void follow_light(glm::vec2 ship_position, glm::vec2 target_position);

    float m_speed = 0.0f;
    float m_direction = 0.0f;
};
