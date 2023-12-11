#pragma once

#include "Light.h"

class DirectionalLight final : public Light
{
public:
    static std::shared_ptr<DirectionalLight> create();

    std::string get_name() const override;

    DirectionalLight() : Light() { }
};
