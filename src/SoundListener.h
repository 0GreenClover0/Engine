#pragma once

#include "Component.h"

class SoundListener final : public Component
{
public:
    static std::shared_ptr<SoundListener> create();

    void update() override;

    inline static std::shared_ptr<SoundListener> instance;
};
