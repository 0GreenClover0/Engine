#pragma once

#include "AK/Badge.h"
#include "Component.h"
#include "Water.h"

class Port;
class LighthouseLight;
class LighthouseKeeper;

class Lighthouse final : public Component
{
public:
    static std::shared_ptr<Lighthouse> create();

    explicit Lighthouse(AK::Badge<Lighthouse>);

    virtual void awake() override;
    virtual void update() override;
    virtual void start() override;
#if EDITOR
    virtual void draw_editor() override;
#endif

    void turn_light(bool const value) const;
    bool is_keeper_inside() const;

    void enter();
    void exit();

    NON_SERIALIZED
    float enterable_distance = 1.0f;

    std::weak_ptr<LighthouseLight> light = {};
    std::weak_ptr<Water> water = {};

    std::weak_ptr<Entity> spawn_position = {};
    std::shared_ptr<Entity> keeper = {};

    NON_SERIALIZED
    bool is_entering_lighthouse_allowed = true;

private:
    bool m_is_keeeper_inside = true;
    bool m_has_keeper_entered_this_frame = false;
};
