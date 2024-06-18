#pragma once

#include "AK/Badge.h"
#include "Component.h"
#include "Water.h"

class Floater final : public Component
{
public:
    static std::shared_ptr<Floater> create();
    static std::shared_ptr<Floater> create(std::weak_ptr<Water> const& water, float const sink, float const side_floaters_offset,
                                           float const side_rotation_strength, float const forward_rotation_strength,
                                           float const forward_floaters_offset);
    explicit Floater(AK::Badge<Floater>);

#if EDITOR
    virtual void draw_editor() override;
#endif
    virtual void awake() override;
    virtual void update() override;

    float sink = 0.01f;

    float side_floaters_offset = 0.1f;
    float side_roation_strength = 5.0f;

    float forward_rotation_strength = 5.0f;
    float forward_floaters_offest = 0.1f;

    std::weak_ptr<Water> water = {};

private:
    float m_previous_height = {};
};
