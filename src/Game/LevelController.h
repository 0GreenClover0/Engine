#pragma once

#include "Component.h"
#include "Curve.h"
#include "Engine.h"
#include "Port.h"

class ScreenText;
class Factory;

class LevelController final : public Component
{
public:
    static std::shared_ptr<LevelController> create();

    static std::shared_ptr<LevelController> get_instance();

    virtual void uninitialize() override;

    virtual void awake() override;
    virtual void update() override;
    virtual void draw_editor() override;

    void on_lighthouse_upgraded() const;

    float map_time = 180.0f;
    u32 map_food = 20;
    i32 maximum_lighthouse_level = 10;
    NON_SERIALIZED
    float time = 0.0f;

    std::vector<std::weak_ptr<Factory>> factories = {};
    std::weak_ptr<Port> port = {};

    float playfield_width = 6.5f;
    float playfield_additional_width = 2.0f;
    float playfield_height = 4.8f;
    float playfield_y_shift = -2.1f;

    std::weak_ptr<Curve> ships_limit_curve = {};
    u32 ships_limit = 0;

    std::weak_ptr<Curve> ships_speed_curve = {};
    float ships_speed = 0.0f;

    std::weak_ptr<Curve> ships_range_curve = {};
    std::weak_ptr<Curve> ships_turn_curve = {};
    std::weak_ptr<Curve> ships_additional_speed_curve = {};
    std::weak_ptr<Curve> pirates_in_control_curve = {};

private:
    inline static std::shared_ptr<LevelController> m_instance;

    std::weak_ptr<ScreenText> m_text = {};
};
