#pragma once

#include "Component.h"
#include "AK/Badge.h"

#include "Serialization.h"

class LighthouseLight;
class LighthouseKeeper;

class Lighthouse final : public Component
{
public:
    static std::shared_ptr<Lighthouse> create();

    explicit Lighthouse(AK::Badge<Lighthouse>);

    virtual void awake() override;
    virtual void update() override;
    virtual void draw_editor() override;

    void enter();
    void exit();

    float enterable_distance = 0.6f;

    std::weak_ptr<LighthouseLight> light = {};

    NON_SERIALIZED
    std::weak_ptr<Entity> spawn_position = {};

private:
    bool m_is_keeeper_inside = true;
};
