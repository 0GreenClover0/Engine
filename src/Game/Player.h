#pragma once

#include "Component.h"
#include "Engine.h"
#include <ScreenText.h>

class Player final : public Component
{
public:
    static std::shared_ptr<Player> create();

    static std::shared_ptr<Player> get_instance();

    Player(AK::Badge<Player>);

    virtual void awake() override;
    virtual void update() override;
    virtual void draw_editor() override;
    virtual void uninitialize() override;

    void upgrade_lighthouse();

    NON_SERIALIZED
    i32 food = 0;
    NON_SERIALIZED
    i32 flash = 0;
    NON_SERIALIZED
    float flash_counter = 0.0f;
    NON_SERIALIZED
    i32 packages = 2;
    NON_SERIALIZED
    i32 lighthouse_level = 0;
    NON_SERIALIZED
    float range = 0.0f;
    NON_SERIALIZED
    float additional_ship_speed = 0.2f * 0.005f;
    NON_SERIALIZED
    float turn_speed = 0.0f;
    NON_SERIALIZED
    float pirates_in_control = 1.0f;

private:
    inline static std::shared_ptr<Player> m_instance;

    float const m_flash_time = 8.3f;

    std::weak_ptr<ScreenText> m_text = {};
};
