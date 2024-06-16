#pragma once

#include "AK/Badge.h"
#include "Component.h"
#include "Water.h"

struct FloaterSettings
{
    float sink_rate = 0.02f;
    float side_rotation_strength = 5.0f;
    float forward_rotation_strength = 5.0f;
    float side_floaters_offset = 0.1f;
    float forward_floaters_offset = 0.1f;
};

class FloatersManager final : public Component
{
public:
    static std::shared_ptr<FloatersManager> create();
    explicit FloatersManager(AK::Badge<FloatersManager>);

    virtual void draw_editor() override;

    FloaterSettings big_boat_settings = {};
    FloaterSettings small_boat_settings = {};
    FloaterSettings medium_boat_settings = {};
    FloaterSettings tool_boat_settings = {};
    FloaterSettings pirate_boat_settings = {};

    std::weak_ptr<Water> water = {};
};
