#pragma once

#include "Component.h"
#include "Engine.h"
#include "Input.h"
#include "Sphere.h"
#include "SpotLight.h"
#include "Window.h"

class Ship;

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

    void set_spot_light(std::shared_ptr<SpotLight> const& light);
    glm::vec2 get_position() const;

    NON_SERIALIZED
    std::weak_ptr<Ship> controlled_ship = {};

    std::weak_ptr<SpotLight> spotlight = {};
    float spotlight_beam_width = 0.15f;

private:
    std::weak_ptr<Sphere> m_sphere = {};
};
