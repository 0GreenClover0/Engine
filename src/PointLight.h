#pragma once

#include "Light.h"

class PointLight final : public Light
{
public:
    static std::shared_ptr<PointLight> create();
    PointLight() : Light() { }

    void draw_editor() override;
    std::string get_name() const override;

    // Default values for an around 50m distance of cover
    float constant = 1.0f; // Should not be changed
    float linear = 0.09f;
    float quadratic = 0.032f;
};
