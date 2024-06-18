#pragma once

#include "Component.h"
#include "Engine.h"
#include "Sprite.h"

class ExampleUIBar final : public Component
{
public:
    static std::shared_ptr<ExampleUIBar> create();

    virtual void awake() override;
    virtual void update() override;
#if EDITOR
    virtual void draw_editor() override;
#endif

    float value = 0.0f;

private:
    std::shared_ptr<Entity> m_sprite_background = nullptr;
    std::shared_ptr<Entity> m_sprite_value = nullptr;
};
