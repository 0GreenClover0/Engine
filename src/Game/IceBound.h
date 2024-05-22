#pragma once

#include "Component.h"

class IceBound final : public Component
{
public:
    static std::shared_ptr<IceBound> create();

    explicit IceBound(AK::Badge<IceBound>);

    virtual void awake() override;
    virtual void update() override;
};
