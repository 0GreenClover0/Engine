#pragma once

#include "Component.h"
#include "Engine.h"
#include <ScreenText.h>

class Player final : public Component
{
public:
    static std::shared_ptr<Player> create();

    static std::shared_ptr<Player> get_instance();

    explicit Player(AK::Badge<Player>);

    virtual void awake() override;
    virtual void update() override;
#if EDITOR
    virtual void draw_editor() override;
#endif
    virtual void uninitialize() override;

    void reset_player();

    void upgrade_lighthouse();

    std::weak_ptr<ScreenText> packages_text = {};
    std::weak_ptr<ScreenText> flashes_text = {};
    std::weak_ptr<ScreenText> level_text = {};
    std::weak_ptr<ScreenText> clock_text = {};
    std::weak_ptr<Entity> progress_bar = {};

    NON_SERIALIZED
    i32 food = 0;
    NON_SERIALIZED
    i32 flash = 0;
    NON_SERIALIZED
    float flash_counter = 0.0f;
    NON_SERIALIZED
    i32 packages = 0;
    NON_SERIALIZED
    i32 lighthouse_level = 0;
    NON_SERIALIZED
    u32 destroyed_ships = 0;

    NON_SERIALIZED
    float range = 0.0f;
    NON_SERIALIZED
    float additional_ship_speed = 0.0f;
    NON_SERIALIZED
    float turn_speed = 0.0f;
    NON_SERIALIZED
    float pirates_in_control = 0.0f;

private:
    inline static std::shared_ptr<Player> m_instance;

    float progress_bar_value = 0.0f;
    u32 m_map_food_helper_variable = 0;
    float const m_flash_time = 8.3f;
};
