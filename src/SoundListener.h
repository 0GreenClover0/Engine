#pragma once

#include "Component.h"
#include "AK/Badge.h"

class SoundListener final : public Component
{
public:
    static std::shared_ptr<SoundListener> create();

    explicit SoundListener(AK::Badge<SoundListener>) {}

    void update() override;

    inline static std::shared_ptr<SoundListener> instance;
};
