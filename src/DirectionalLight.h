#pragma once

#include "Light.h"

class DirectionalLight final : public Light
{
public:
    static std::shared_ptr<DirectionalLight> create();
    DirectionalLight() : Light() { }
};
