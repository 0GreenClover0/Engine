#pragma once

#include <glm/vec3.hpp>

#include "Component.h"

class Light : public Component
{
public:
    ~Light() override = 0;

    virtual void draw_editor() override;

    virtual void initialize() override;
    virtual void uninitialize() override;

    virtual void on_enabled() override;
    virtual void on_disabled() override;

    glm::vec3 ambient = { 0.2f, 0.2f, 0.2f };
    glm::vec3 diffuse = { 1.0f, 1.0f, 1.0f };
    glm::vec3 specular = { 1.0f, 1.0f, 1.0f };

protected:
    Light() = default;
};
