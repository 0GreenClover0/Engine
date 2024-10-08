#pragma once

#include <vector>

#include "Collider2D.h"

enum class CollisionType
{
    Undefined,
    CircleCircle,
    RectangleRectangle,
    CircleRectangle
};

class PhysicsEngine
{
public:
    PhysicsEngine() = default;

    static void set_instance(std::shared_ptr<PhysicsEngine> const& scene)
    {
        m_instance = scene;
    }

    static std::shared_ptr<PhysicsEngine> get_instance()
    {
        return m_instance;
    }

    PhysicsEngine(PhysicsEngine const&) = delete;
    void operator=(PhysicsEngine const&) = delete;

    void initialize();
    void run_updates();

    static void on_collision_enter(std::shared_ptr<Collider2D> const& collider, std::shared_ptr<Collider2D> const& other);
    static void on_collision_exit(std::shared_ptr<Collider2D> const& collider, std::shared_ptr<Collider2D> const& other);

    static void on_trigger_enter(std::shared_ptr<Collider2D> const& collider, std::shared_ptr<Collider2D> const& other);
    static void on_trigger_exit(std::shared_ptr<Collider2D> const& collider, std::shared_ptr<Collider2D> const& other);

    bool is_collider_registered(std::shared_ptr<Collider2D> const& collider) const;
    void emplace_collider(std::shared_ptr<Collider2D> const& collider);
    void remove_collider(std::shared_ptr<Collider2D> const& collider);

    static bool compute_penetration(std::shared_ptr<Collider2D> const& collider, std::shared_ptr<Collider2D> const& other, glm::vec2& mtv);

private:
    void update_physics() const;
    void solve_collisions() const;

    static bool test_collision_rectangle_rectangle(Collider2D const& obb1, Collider2D const& obb2, glm::vec2& mtv);
    static bool test_collision_circle_circle(Collider2D const& obb1, Collider2D const& obb2, glm::vec2& mtv);
    static bool test_collision_circle_rectangle(Collider2D const& circle_collider, Collider2D const& rect_collider, glm::vec2& mtv);

    static bool compute_penetration_circle_segment(glm::vec2 const& center, float const radius, glm::vec2 const& p1, glm::vec2 const& p2,
                                                   glm::vec2& mtv);

    static bool is_point_inside_obb(glm::vec2 const& point, std::array<glm::vec2, 4> const& rectangle_corners);

    std::vector<std::shared_ptr<Collider2D>> colliders = {};

    double m_accumulated_delta = 0.0;

    inline static std::shared_ptr<PhysicsEngine> m_instance;
};
