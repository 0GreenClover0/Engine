#pragma once

#include "Component.h"
#include "Engine.h"
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

private:

    std::weak_ptr<Sphere> m_sphere = {};
};