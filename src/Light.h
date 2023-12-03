#pragma once

#include <glm/vec3.hpp>

#include "Component.h"

class Light : public Component
{
public:
    static std::shared_ptr<Light> create();
    Light() = default;
};
