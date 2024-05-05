#pragma once

#include "Component.h"
#include "Engine.h"

class ScreenText;
class Factory;

class GameController final : public Component
{
public:
    static std::shared_ptr<GameController> create();

    static std::shared_ptr<GameController> get_instance();

    virtual void uninitialize() override;

    virtual void awake() override;
    virtual void update() override;
    virtual void draw_editor() override;

    NON_SERIALIZED
    i32 food = 0;
    NON_SERIALIZED
    i32 flash = 0;
    NON_SERIALIZED
    float flash_counter = 0.0f;
    float flash_time = 8.3f;
    NON_SERIALIZED
    i32 packages = 2;
    NON_SERIALIZED
    i32 lighthouse_level = 0;

    float map_time = 0.0f;
    NON_SERIALIZED
    float time = 0.0f;

    std::vector<std::weak_ptr<Factory>> factories = {};

private:
    inline static std::shared_ptr<GameController> m_instance;

    std::weak_ptr<ScreenText> m_text = {};
};
