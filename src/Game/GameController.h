#pragma once

#include "Component.h"
#include "Engine.h"
#include "Serialization.h"

class GameController final : public Component
{
public:
    static std::shared_ptr<GameController> create();

    virtual void awake() override;
    virtual void update() override;
    virtual void draw_editor() override;

    NON_SERIALIZED
    i32 food = 0;
    NON_SERIALIZED
    i32 flash = 0;
    NON_SERIALIZED
    i32 packages = 0;

    float map_time = 0.0f;
    NON_SERIALIZED
    float time = 0.0f;
};
