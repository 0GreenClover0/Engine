#pragma once

#include "Light.h"
#include "AK/Badge.h"

class DirectionalLight final : public Light
{
public:
    static std::shared_ptr<DirectionalLight> create();

    virtual void draw_editor() override;

    virtual std::string get_name() const override;

    explicit DirectionalLight(AK::Badge<DirectionalLight> badge) : Light() { }
};
