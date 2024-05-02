#pragma once

#include "Component.h"
#include "LighthouseLight.h"

class Ship final : public Component
{
public:
    static std::shared_ptr<Ship> create();
    static std::shared_ptr<Ship> create(std::shared_ptr<LighthouseLight> const& light);

    explicit Ship(AK::Badge<Ship>);

    void set_light(std::shared_ptr<LighthouseLight> const& light);

    virtual void awake() override;
    virtual void update() override;
    virtual void draw_editor() override;

    float minimum_speed = 0.35f * 0.005f;
    float maximum_speed = 0.75f * 0.005f;

    float turn_speed = 0.25f;
    i32 visibility_range = 110;

private:
    void follow_light(glm::vec2 ship_position, glm::vec2 target_position);

    float m_speed = 0.0f;
    float m_direction = 0.0f;
    std::weak_ptr<LighthouseLight> m_light;
};
