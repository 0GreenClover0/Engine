#pragma once

#include <glm/vec3.hpp>

#include "Component.h"
#include "AK/Badge.h"

class Light : public Component
{
public:
    static std::shared_ptr<Light> create();
    explicit Light(AK::Badge<Light>) {}

    virtual void draw_editor() override;
    virtual std::string get_name() const override;

    virtual void on_enabled() override;
    virtual void on_disabled() override;

    glm::vec3 ambient = { 0.2f, 0.2f, 0.2f };
    glm::vec3 diffuse = { 1.0f, 1.0f, 1.0f };
    glm::vec3 specular = { 1.0f, 1.0f, 1.0f };

protected:
    Light() = default;
};
