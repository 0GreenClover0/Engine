#pragma once

#include "Component.h"
#include "Serialization.h"
#include "Input.h"

class LighthouseKeeper final : public Component
{
public:
    static std::shared_ptr<LighthouseKeeper> create();

    explicit LighthouseKeeper(AK::Badge<LighthouseKeeper>);

    virtual void awake() override;
    virtual void update() override;
    virtual void draw_editor() override;

    float maximum_speed = 0.08f;
    float acceleration = maximum_speed / 23.0f;
    float deceleration = acceleration;

private:
    glm::vec2 m_speed = glm::vec2(0.0f, 0.0f);
};
