#pragma once

#include "Component.h"
#include "Serialization.h"
#include "Input.h"

class Factory;
class Lighthouse;

class LighthouseKeeper final : public Component
{
public:
    static std::shared_ptr<LighthouseKeeper> create();

    explicit LighthouseKeeper(AK::Badge<LighthouseKeeper>);

    virtual void awake() override;
    virtual void update() override;
    virtual void draw_editor() override;

    float maximum_speed = 3.17f;
    float acceleration = 0.14f;
    float deceleration = acceleration;

    float interact_with_factory_distance = 0.5f;

    std::weak_ptr<Lighthouse> lighthouse = {};

private:
    void handle_input() const;

    glm::vec2 m_speed = glm::vec2(0.0f, 0.0f);
};
