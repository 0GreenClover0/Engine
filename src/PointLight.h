#pragma once

#include "Light.h"

class PointLight final : public Light
{
public:
    static std::shared_ptr<PointLight> create();
    PointLight() : Light() { }
};
