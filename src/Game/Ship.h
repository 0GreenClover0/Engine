#pragma once

#include "Component.h"
#include "LevelController.h"
#include "LighthouseLight.h"
#include "ShipEyes.h"

class Floater;

enum class ShipType
{
    FoodSmall,
    FoodMedium,
    FoodBig,
    Pirates,
    Tool
};

enum class BehavioralState
{
    Normal,
    Pirate,
    Control,
    Avoid,
    Destroyed,
    InPort,
    Stop,
};

#if EDITOR
static void ship_type_to_color(ShipType const type, ImVec4& bg_color, ImVec4& text_color)
{
    auto constexpr light_green_color = ImVec4(0.0f, 0.91f, 0.118f, 1.0f);
    auto constexpr green_color = ImVec4(0.0f, 0.729f, 0.118f, 1.0f);
    auto constexpr dark_green_color = ImVec4(0.0f, 0.549f, 0.118f, 1.0f);
    auto constexpr gray_color = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
    auto constexpr pink_color = ImVec4(0.671f, 0.2f, 0.62f, 1.0f);
    auto constexpr white_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    auto constexpr black_color = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);

    switch (type)
    {
    case ShipType::FoodSmall:
        bg_color = light_green_color;
        text_color = black_color;
        break;
    case ShipType::FoodMedium:
        bg_color = green_color;
        text_color = black_color;
        break;
    case ShipType::FoodBig:
        bg_color = dark_green_color;
        text_color = black_color;
        break;
    case ShipType::Pirates:
        bg_color = gray_color;
        text_color = white_color;
        break;
    case ShipType::Tool:
        bg_color = pink_color;
        text_color = white_color;
        break;
    default:
        bg_color = white_color;
        text_color = black_color;
        break;
    }
}
#endif

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

static float ship_type_to_range_factor(ShipType const type)
{
    switch (type)
    {
    case ShipType::FoodSmall:
        return 0.55f;

    case ShipType::FoodMedium:
        return 0.75f;

    case ShipType::FoodBig:
        return 1.0f;

    case ShipType::Pirates:
        return 0.55f;

    case ShipType::Tool:
        return 0.55f;

    default:
        return 1.0f;
    }
}

static std::string behaviour_state_to_string(BehavioralState const state)
{
    switch (state)
    {
    case BehavioralState::Normal:
        return "Normal";

    case BehavioralState::Pirate:
        return "Pirate";

    case BehavioralState::Control:
        return "Control";

    case BehavioralState::Avoid:
        return "Avoid";

    case BehavioralState::Destroyed:
        return "Destroyed";

    case BehavioralState::InPort:
        return "InPort";

    case BehavioralState::Stop:
        return "Stop";

    default:
        return "Undefined state";
    }
}

class ShipSpawner;

class Ship final : public Component
{
public:
    static std::shared_ptr<Ship> create();
    static std::shared_ptr<Ship> create(std::shared_ptr<LighthouseLight> const& light, std::shared_ptr<ShipSpawner> const& spawner,
                                        std::shared_ptr<ShipEyes> const& eyes);

    explicit Ship(AK::Badge<Ship>);

    virtual void awake() override;
    virtual void update() override;
    virtual void on_destroyed() override;
#if EDITOR
    virtual void draw_editor() override;
#endif

    virtual void on_trigger_enter(std::shared_ptr<Collider2D> const& other) override;
    virtual void on_trigger_exit(std::shared_ptr<Collider2D> const& other) override;

    void set_start_direction();
    void destroy(std::shared_ptr<Entity> const& other_entity_collider);
    void stop();
    bool is_in_port() const;
    void set_direction(float direction);

    NON_SERIALIZED
    float minimum_speed = 0.11f;
    NON_SERIALIZED
    float maximum_speed = 0.23f;

    ShipType type = ShipType::FoodSmall;

    std::weak_ptr<LighthouseLight> light = {};
    std::weak_ptr<ShipSpawner> spawner = {};
    std::weak_ptr<ShipEyes> eyes = {};
    std::weak_ptr<PointLight> my_light = {};

    NON_SERIALIZED
    bool is_destroyed = false;

    NON_SERIALIZED
    std::weak_ptr<Floater> floater = {};

    Event<void(std::shared_ptr<Ship>)> on_ship_destroyed;

    NON_SERIALIZED
    BehavioralState behavioral_state = BehavioralState::Normal;

    NON_SERIALIZED
    bool is_in_flash_collider = false;

private:
    void follow_point(glm::vec2 ship_position, glm::vec2 target_position);

    void update_position() const;
    void update_rotation() const;

    bool normal_state_change();
    bool pirate_state_change();
    bool control_state_change();
    bool avoid_state_change();
    bool destroyed_state_change();
    bool in_port_state_change();

    bool control_state_ended();
    bool avoid_state_ended() const;

    void normal_behavior();
    void pirate_behavior();
    void control_behavior();
    void avoid_behavior();
    void destroyed_behavior();
    void in_port_behavior();
    void stop_behavior();

    bool is_out_of_room() const;

    float m_speed = 0.0f;
    float m_direction = 0.0f;
    float m_range_factor = 1.0f;

    float const m_how_deep_sink_factor = 0.26f;

    bool m_is_in_port = false;

    float m_collision_rotation_counter = 0.0f;
    float m_destroyed_counter = 0.0f;
    float m_scale_down_counter = 0.0f;

    i32 const m_visibility_range = 110;
    float const m_start_direction_wiggle = 15.0f;

    float const m_collision_rotation_time = 0.5f;
    float const m_destroy_time = 3.5f;
    float const m_scale_down_time = 1.0f;
    float const m_deceleration_speed = 0.17f;

    float m_pirates_in_control_counter = 0.0f;
    glm::quat target_rotation_after_collision = {};
    glm::quat rotation_before_collision = {};
    i32 m_avoid_direction = 0;
};
