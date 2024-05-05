#pragma once

#include "Component.h"
#include "Engine.h"
#include "Serialization.h"
#include "Input.h"
#include "Sphere.h"
#include "Window.h"

class LighthouseLight final : public Component
{
public:
    static std::shared_ptr<LighthouseLight> create();

    explicit LighthouseLight(AK::Badge<LighthouseLight>);

    virtual void on_enabled() override;
    virtual void on_disabled() override;

    virtual void awake() override;
    virtual void update() override;
    virtual void draw_editor() override;

    glm::vec2 get_position() const;

    float range = 100.0f * 0.005f;
    float additional_ship_speed = 0.2f * 0.005f;

    float const playfield_width = 5.4f;
    float const playfield_additional_width = 1.4f;
    float const playfield_height = 3.6f;

private:

    std::weak_ptr<Sphere> m_sphere = {};
};
