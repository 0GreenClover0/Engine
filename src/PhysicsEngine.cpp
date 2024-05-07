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

void PhysicsEngine::on_collision_exit(std::shared_ptr<Collider2D> const &collider, std::shared_ptr<Collider2D> const& other)
{
    for (auto const& component : collider->entity->components)
    {
        component->on_collision_exit(other);
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

            bool const should_overlap_as_trigger = collider1->is_trigger() || collider2->is_trigger() && collider1->is_trigger() != collider2->is_trigger();

            // CIRCLE/CIRCLE
            if (collider1_type == collider2_type && collider1_type == ColliderType2D::Circle && collider1->overlaps(*collider2))
            {
                if (should_overlap_as_trigger)
                {
                    Debug::log("Overlap");
                }
                else
                {
                    on_collision_enter(collider1, collider2);
                    on_collision_enter(collider2, collider1);

                    if (!collider1->is_static())
                        collider1->separate(*collider2);

                    if (!collider2->is_static())
                        collider2->separate(*collider1);

                    on_collision_exit(collider1, collider2);
                    on_collision_exit(collider2, collider1);
                }
            }

            // RECTANGLE/RECTANGLE
            else if (collider1_type == collider2_type && collider1_type == ColliderType2D::Rectangle && collider1->overlaps(*collider2))
            {
                if (should_overlap_as_trigger)
                {
                    Debug::log("Overlap");
                }
                else
                {
                    on_collision_enter(collider1, collider2);
                    on_collision_enter(collider2, collider1);

                    if (!collider1->is_static())
                        collider1->separate(true);

                    if (!collider2->is_static())
                        collider2->separate(true);

                    on_collision_exit(collider1, collider2);
                    on_collision_exit(collider2, collider1);
                }
            }

            // CIRCLE/RECTANGLE
            else if (collider1_type != collider2_type)
            {
                if (collider1->overlaps(*collider2))
                {
                    if (should_overlap_as_trigger)
                    {
                        Debug::log("Overlap");
                    }
                    else
                    {
                        on_collision_enter(collider1, collider2);
                        on_collision_enter(collider2, collider1);

                        if (!collider1->is_static())
                            collider1->separate(true);

                        if (!collider2->is_static())
                            collider2->separate(true);

                        on_collision_exit(collider1, collider2);
                        on_collision_exit(collider2, collider1);
                    }
                }
            }
        }
    }
}
