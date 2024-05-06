#pragma once

#include "Component.h"
#include "LighthouseLight.h"
#include "LevelController.h"

enum class ShipType
{
    FoodSmall,
    FoodMedium,
    FoodBig,
    Pirates,
    Tool
};

static std::string ship_type_to_string(ShipType const type)
{
    switch (type)
    {
    case ShipType::FoodSmall:
        return "Food small";

    case ShipType::FoodMedium:
        return "Food medium";

    case ShipType::FoodBig:
        return "Food big";

    case ShipType::Pirates:
        return "Pirates";

    case ShipType::Tool:
        return "Tool";

    default:
        return "Undefined ship";
    }
}

class ShipSpawner;

class Ship final : public Component
{
public:
    static std::shared_ptr<Ship> create();
    static std::shared_ptr<Ship> create(std::shared_ptr<LighthouseLight> const& light, std::shared_ptr<ShipSpawner> const& spawner);

    explicit Ship(AK::Badge<Ship>);

    virtual void awake() override;
    virtual void update() override;
    virtual void on_destroyed() override;
    virtual void draw_editor() override;

    void destroy();
    void stop();
    bool is_in_port() const;

    static void on_lighthouse_upgraded(float _turn_speed, float _range, float _additional_ship_speed, float _pirates_in_control);

    NON_SERIALIZED
    float minimum_speed = 0.11f;
    NON_SERIALIZED
    float maximum_speed = 0.23f;
    NON_SERIALIZED
    inline static float turn_speed = 15.0f;
    NON_SERIALIZED
    inline static float range = 100.0f * 0.005f;
    NON_SERIALIZED
    inline static float additional_ship_speed = 0.2f * 0.005f;
    NON_SERIALIZED
    inline static float pirates_in_control = 10.0f;

    NON_SERIALIZED
    ShipType type = ShipType::FoodSmall;

    std::weak_ptr<LighthouseLight> light = {};
    std::weak_ptr<ShipSpawner> spawner = {};

    NON_SERIALIZED
    bool is_destroyed = false;

    Event<void(std::shared_ptr<Ship>)> on_ship_destroyed;

private:
    void follow_point(glm::vec2 ship_position, glm::vec2 target_position);

    bool is_out_of_room() const;

    float m_speed = 0.0f;
    float m_direction = 0.0f;
    
    float const m_how_deep_sink_factor = 0.26f;

    bool m_is_in_port = false;

    float m_destroyed_counter = 0.0f;

    i32 const m_visibility_range = 110;
    float const m_start_direction_wiggle = 15.0f;
    float const m_destroy_time = 6.5f;
    float const m_deceleration_speed = 0.17f;

    float m_pirates_in_control_counter = 0.0f;
};
