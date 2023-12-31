#pragma once

#include "Light.h"

#include <glm/trigonometric.hpp>

class SpotLight final : public Light
{
public:
    static std::shared_ptr<SpotLight> create();
    SpotLight() : Light() { }

    void draw_editor() override;
    std::string get_name() const override;

    // Default values for an around 50m distance of cover
    float constant = 1.0f; // Should not be changed
    float linear = 0.09f;
    float quadratic = 0.032f;

    float cut_off = glm::cos(glm::radians(32.5f));
    float outer_cut_off = glm::cos(glm::radians(60.0f));
};
