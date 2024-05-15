#include "PhysicsEngine.h"

#include "Debug.h"
#include "Engine.h"
#include "Entity.h"
#include "AK/AK.h"

void PhysicsEngine::initialize()
{
    auto const physics_engine = std::make_shared<PhysicsEngine>();
    set_instance(physics_engine);
}

void PhysicsEngine::update_physics()
{
    solve_collisions();
}

void PhysicsEngine::on_collision_enter(std::shared_ptr<Collider2D> const& collider, std::shared_ptr<Collider2D> const& other)
{
    for (auto const& component : collider->entity->components)
    {
        component->on_collision_enter(other);
    }
}

void PhysicsEngine::on_collision_exit(std::shared_ptr<Collider2D> const& collider, std::shared_ptr<Collider2D> const& other)
{
    for (auto const& component : collider->entity->components)
    {
        component->on_collision_exit(other);
    }
}

void PhysicsEngine::on_trigger_enter(std::shared_ptr<Collider2D> const& collider,
    std::shared_ptr<Collider2D> const& other)
{
    for (auto const& component : collider->entity->components)
    {
        component->on_trigger_enter(other);
    }
}

void PhysicsEngine::on_trigger_exit(std::shared_ptr<Collider2D> const& collider,
    std::shared_ptr<Collider2D> const& other)
{
    for (auto const& component : collider->entity->components)
    {
        component->on_trigger_exit(other);
    }
}

void PhysicsEngine::emplace_collider(std::shared_ptr<Collider2D> const& collider)
{
    colliders.emplace_back(collider);
}

void PhysicsEngine::remove_collider(std::shared_ptr<Collider2D> const& collider)
{
    AK::swap_and_erase(colliders, collider);
}

void PhysicsEngine::solve_collisions() const
{
    // Collision detection
    for (u32 i = 0; i < colliders.size(); i++)
    {
        for (u32 j = 0; j < colliders.size(); j++)
        {
            if (i == j || (colliders[i]->is_static() && colliders[j]->is_static()))
                continue;

            std::shared_ptr<Collider2D> collider1 = colliders[i];
            std::shared_ptr<Collider2D> collider2 = colliders[j];
            ColliderType2D const collider1_type = colliders[i]->get_collider_type();
            ColliderType2D const collider2_type = colliders[j]->get_collider_type();

            CollisionInfo ci_1 = {};
            CollisionInfo ci_2 = {};
            CollisionInfo ci_3 = {};

            bool const should_overlap_as_trigger = collider1->is_trigger() || collider2->is_trigger();

            auto collision_type = CollisionType::Undefined;

            if (collider1_type == collider2_type && collider1_type == ColliderType2D::Circle)
                collision_type = CollisionType::CircleCircle;

            if (collider1_type == collider2_type && collider1_type == ColliderType2D::Rectangle)
                collision_type = CollisionType::RectangleRectangle;

            if (collider1_type != collider2_type)
                collision_type = CollisionType::CircleRectangle;

            switch (collision_type)
            {
            case CollisionType::CircleCircle:
                ci_1 = collider1->overlaps(*collider2);
                if (ci_1.is_overlapping)
                {
                    if (should_overlap_as_trigger)
                    {
                        collider1->add_overlapped_this_frame(collider2);

#if _DEBUG
                        if (collider1->is_inside_trigger(collider2->guid))
                        {
                            assert(collider2->is_inside_trigger(collider1->guid));
                        }
                        else
                        {
                            assert(!collider2->is_inside_trigger(collider1->guid));
                        }
#endif
                    }
                    else
                    {
                        on_collision_enter(collider1, collider2);
                        on_collision_enter(collider2, collider1);

                        if (!collider1->is_static() && !collider2->is_static())
                        {
                            collider1->apply_mtv(true, ci_1);
                            collider2->apply_mtv(false, ci_1);
                        }
                        else if (collider1->is_static())
                        {
                            collider2->apply_mtv(false, ci_1);
                        }
                        else if (collider2->is_static())
                        {
                            collider1->apply_mtv(true, ci_1);
                        }

                        on_collision_exit(collider1, collider2);
                        on_collision_exit(collider2, collider1);
                    }

                    break;

            case CollisionType::RectangleRectangle:
                ci_2 = collider1->overlaps(*collider2);
                if (ci_2.is_overlapping)
                {
                    if (should_overlap_as_trigger)
                    {
                        collider1->add_overlapped_this_frame(collider2);

#if _DEBUG
                        if (collider1->is_inside_trigger(collider2->guid))
                        {
                            assert(collider2->is_inside_trigger(collider1->guid));
                        }
                        else
                        {
                            assert(!collider2->is_inside_trigger(collider1->guid));
                        }
#endif
                    }
                    else
                    {
                        on_collision_enter(collider1, collider2);
                        on_collision_enter(collider2, collider1);

                        if (!collider1->is_static() && !collider2->is_static())
                        {
                            collider1->apply_mtv(true, ci_2);
                            collider2->apply_mtv(false, ci_2);
                        }
                        else if (collider1->is_static())
                        {
                            collider2->apply_mtv(false, ci_2);
                        }
                        else if (collider2->is_static())
                        {
                            collider1->apply_mtv(true, ci_2);
                        }

                        on_collision_exit(collider1, collider2);
                        on_collision_exit(collider2, collider1);
                    }
                }
                break;

            case CollisionType::CircleRectangle:
                ci_3 = collider1->overlaps(*collider2);
                if (ci_3.is_overlapping)
                {
                    if (should_overlap_as_trigger)
                    {
                        collider1->add_overlapped_this_frame(collider2);

#if _DEBUG
                        if (collider1->is_inside_trigger(collider2->guid))
                        {
                            assert(collider2->is_inside_trigger(collider1->guid));
                        }
                        else
                        {
                            assert(!collider2->is_inside_trigger(collider1->guid));
                        }
#endif
                    }
                    else
                    {
                        on_collision_enter(collider1, collider2);
                        on_collision_enter(collider2, collider1);

                        if (!collider1->is_static() && !collider2->is_static())
                        {
                            collider1->apply_mtv(true, ci_3);
                            collider2->apply_mtv(false, ci_3);
                        }
                        else if (collider1->is_static())
                        {
                            collider2->apply_mtv(false, ci_3);
                        }
                        else if (collider2->is_static())
                        {
                            collider1->apply_mtv(true, ci_3);
                        }

                        on_collision_exit(collider1, collider2);
                        on_collision_exit(collider2, collider1);
                    }
                }
                break;

            case CollisionType::Undefined:
            default:
                Debug::log("Undefined collision detected", DebugType::Error);
                std::unreachable();
                }
            }
        }
    }

    for (auto const& collider : colliders)
    {
        std::unordered_map<std::string, std::weak_ptr<Collider2D>> new_inside_trigger = {};
        std::vector<std::weak_ptr<Collider2D>> new_inside_trigger_vector = {};

        for (auto const& other : collider->get_all_overlapping_this_frame())
        {
            if (other.expired())
                continue;

            auto const other_locked = other.lock();

            new_inside_trigger.emplace(other_locked->guid, other);
            new_inside_trigger_vector.emplace_back(other);

            if (!collider->is_inside_trigger(other_locked->guid))
            {
                on_trigger_enter(collider, other_locked);
            }
        }

        for (auto const& other : collider->get_inside_trigger_vector())
        {
            if (other.expired())
                continue;

            auto const other_locked = other.lock();

            if (!new_inside_trigger.contains(other_locked->guid))
            {
                on_trigger_exit(collider, other_locked);
            }
        }

        collider->set_inside_trigger(new_inside_trigger, new_inside_trigger_vector);
        collider->clear_overlapped_this_frame();
    }
}
