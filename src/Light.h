#pragma once

#include <glm/vec3.hpp>

#include "Component.h"

class Light : public Component
{
public:
    static std::shared_ptr<Light> create();
    Light() = default;

    glm::vec3 ambient = glm::vec3(0.1f, 0.1f, 0.1f);
    glm::vec3 diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 specular = glm::vec3(1.0f, 1.0f, 1.0f);
};
