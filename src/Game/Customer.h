#pragma once

#include "Component.h"

#include "AK/Badge.h"

class Curve;

class Customer final : public Component
{
public:
    static std::shared_ptr<Customer> create();

    explicit Customer(AK::Badge<Customer>);

    virtual void awake() override;
    virtual void update() override;
    virtual void on_collision_enter(std::shared_ptr<Collider2D> const& other) override;

#if EDITOR
    virtual void draw_editor() override;
#endif

    void feed(glm::vec3 const& destination);

    void set_destination(glm::vec3 const& destination);

    std::weak_ptr<Collider2D> collider = {};

    std::weak_ptr<Entity> left_hand = {};
    std::weak_ptr<Entity> right_hand = {};

    NON_SERIALIZED
    float desired_height = 0.0f;

private:
    glm::vec3 m_destination = {};
    glm::vec3 m_pushed_destination = {};

    float m_jump_timer_min = 3.0f;
    float m_jump_timer_max = 6.0f;
    float m_jump_timer = 0.0f;

    float m_velocity_y = 0.0f;
    bool m_is_jumping = false;
    bool m_is_waiting_to_jump_to_water = false;

    inline static constexpr float m_spread_arms_min = 10.0f;
    inline static constexpr float m_spread_arms_max = 60.0f;
    float m_spreading_arms_timer = 0.0f;

    float m_spreading_arms_speed = 100.0f;

    bool m_is_spreading_arms = false;
    bool m_is_unspreading_arms = false;

    bool m_is_fed = false;

    glm::vec3 m_desired_rotation = {};
    glm::vec3 m_standing_rotation = {90.0f, -90.0f, -90.0f};

    inline static float constexpr m_max_jump_velocity = 3.0f;

    inline static float constexpr m_max_left_arm_spread = 50.0f;
};
