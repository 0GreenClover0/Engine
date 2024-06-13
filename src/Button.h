#pragma once

#include "AK/Badge.h"
#include "Component.h"

class Button : public Component
{
public:
    static std::shared_ptr<Button> create();
    explicit Button(AK::Badge<Button>);

    virtual void initialize() override;
};
