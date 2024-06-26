#pragma once

#include "AK/Badge.h"
#include "Component.h"

#include <glm/vec2.hpp>

class HovercraftWithoutKeeper final : public Component
{
public:
    static std::shared_ptr<HovercraftWithoutKeeper> create();
    explicit HovercraftWithoutKeeper(AK::Badge<HovercraftWithoutKeeper>);

    virtual void awake() override;
    virtual void update() override;

    NON_SERIALIZED
    glm::vec2 speed = {};

private:
    std::weak_ptr<Collider2D> m_collider = {};
    float m_maximum_speed = 5.0f;
    float m_deceleration = 0.2f;
};
