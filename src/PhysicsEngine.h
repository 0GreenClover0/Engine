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
    void update_physics(); // TODO: All physics calculations should be in some kind of FixedUpdate

    static void on_collision_enter(std::shared_ptr<Collider2D> const& collider, std::shared_ptr<Collider2D> const& other);
    static void on_collision_exit(std::shared_ptr<Collider2D> const& collider, std::shared_ptr<Collider2D> const& other);

    static void on_trigger_enter(std::shared_ptr<Collider2D> const& collider, std::shared_ptr<Collider2D> const& other);
    static void on_trigger_exit(std::shared_ptr<Collider2D> const& collider, std::shared_ptr<Collider2D> const& other);

    void emplace_collider(std::shared_ptr<Collider2D> const& collider);
    void remove_collider(std::shared_ptr<Collider2D> const& collider);

private:
    void solve_collisions() const;
    std::vector<std::shared_ptr<Collider2D>> colliders = {};
    inline static std::shared_ptr<PhysicsEngine> m_instance;
};
